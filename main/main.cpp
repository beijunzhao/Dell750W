/**
 * main.cpp - 台达/戴尔 750W 数控电源控制板主程序
 *
 * ESP32-C3 (ESP-IDF 框架)
 *
 * 架构: FreeRTOS 多任务并发
 *   - task_hardware_poll: 硬件轮询 (PMBus + ADC), 500ms 周期, 优先级 2
 *   - task_ui_and_buttons: UI 与按键处理, 50ms 周期, 优先级 3
 *   - app_main (BLE Task): BLE 状态监控与数据推送, 100ms 周期
 *
 * 安全开机时序 (setup):
 *   1. SW_CTRL = LOW (切断电源输出)
 *   2. V_PWM = 最高占空比 (3.0V DAC → 0V 输出), I_PWM = 0
 *   3. 延时 50ms 等待 RC 滤波器稳定
 *   4. 初始化其他外设 (I2C, ADC, BLE, 按键)
 *
 * 通信协议: JSON over BLE UART (Nordic UART Service)
 *   请求: {"cmd":"get_data"} / {"cmd":"set","V_set":12.0} / {"cmd":"set","power":1}
 *   响应: {"V_out":12.0,...} / {"MFR_ID":"...",...}
 */
#include "pin_map.h"
#include "power_control.h"
#include "adc_sampler.h"
#include "pmbus.h"
#include "ble_server.h"
#include "lcd_display.h"
#include "lvgl_setup.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

static const char* TAG = "Main";

// ---------- 前向声明 ----------
extern "C" void nimble_host_task(void *param);
static void on_ble_rx(const char* data, int len);
static void init_peripherals(void);
static void handle_buttons(void);
static const char* build_full_data_json(void);
static void send_response(const char* json);

// FreeRTOS 任务函数前向声明
static void task_hardware_poll(void *pvParameters);
static void task_ui_and_buttons(void *pvParameters);

// 全局 JSON 缓冲区 (BLE 发送用)
static char g_respBuf[1024];

// BLE RX 分包累积缓冲区 (MTU=23 时数据可能被分片)
// 最大支持 4KB 的 JSON 命令
#define BLE_RX_BUF_SIZE 4096
static char g_bleRxBuf[BLE_RX_BUF_SIZE];
static int g_bleRxLen = 0;

// ---------- app_main 入口 ----------

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, " Dell/Delta 750W PSU Controller v1.0");
    ESP_LOGI(TAG, " Platform: ESP32-C3");
    ESP_LOGI(TAG, "========================================");

    // ========== 安全开机时序 (严禁修改) ==========

    // 第一步: 锁定命门 - SW_CTRL 输出 LOW, V_PWM 最高占空比, I_PWM = 0
    ESP_LOGI(TAG, "[BOOT SEQ] Step 1: Locking SW_CTRL LOW, setting PWM safe bias...");
    esp_err_t ret = PowerControl::init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "FATAL: PowerControl init failed! System halted.");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 时序注释: 此时 V_PWM 输出 90.9% 占空比 (3.0V → 0V 物理输出), I_PWM 输出 0%
    // 硬件安全: 即使 MCU 在此后死机, 外部 RC 滤波器也会维持 V_DAC≈3.0V, I_DAC≈0V

    // 第二步: 延时 50ms, 等待 RC 滤波器电容充电稳定
    ESP_LOGI(TAG, "[BOOT SEQ] Step 2: Waiting 50ms for RC filter stabilization...");
    vTaskDelay(pdMS_TO_TICKS(50));

    // 第三步: 初始化其他外设
    ESP_LOGI(TAG, "[BOOT SEQ] Step 3: Initializing peripherals...");
    init_peripherals();

    // ========== BLE 初始化 ==========
    ESP_LOGI(TAG, "Setting up BLE server...");
    ble_server_set_rx_callback(on_ble_rx);
    ret = ble_server_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "FATAL: BLE init failed!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 启动 NimBLE 主机任务
    ESP_LOGI(TAG, "Starting NimBLE host task...");
    nimble_port_freertos_init(nimble_host_task);

    // ========== 创建 FreeRTOS 任务 ==========

    // 创建硬件轮询任务 (PMBus + ADC), 500ms 周期, 优先级 2
    BaseType_t taskCreated;
    taskCreated = xTaskCreate(
        task_hardware_poll,
        "hw_poll",
        4096,       // 栈空间 4096
        NULL,       // 参数
        2,          // 优先级 2
        NULL        // 任务句柄 (不需要)
    );
    if (taskCreated != pdPASS) {
        ESP_LOGE(TAG, "FATAL: Failed to create hardware poll task!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
    ESP_LOGI(TAG, "Hardware poll task created (prio=2, stack=4096)");

    // 创建 UI 与按键任务, 50ms 周期, 优先级 3
    taskCreated = xTaskCreate(
        task_ui_and_buttons,
        "ui_btn",
        4096,       // 栈空间 4096
        NULL,       // 参数
        3,          // 优先级 3 (高于硬件轮询, 确保交互不卡顿)
        NULL        // 任务句柄 (不需要)
    );
    if (taskCreated != pdPASS) {
        ESP_LOGE(TAG, "FATAL: Failed to create UI/button task!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
    ESP_LOGI(TAG, "UI/button task created (prio=3, stack=4096)");

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, " System ready. Advertising as '%s'", ble_server_get_device_name());
    ESP_LOGI(TAG, " Waiting for BLE connection...");
    ESP_LOGI(TAG, "========================================");

    // ========== 主循环 (BLE Task) ==========
    // 只保留 BLE 状态监控和定时数据推送逻辑
    uint64_t lastDataPush = 0;
    bool firstDataSent = false;
    bool wasConnected = false;

    while (1) {
        uint64_t now = esp_timer_get_time() / 1000; // ms
        bool isConnected = ble_server_is_connected();

        // 检测连接状态变化: 刚连接上时重置 firstDataSent
        if (isConnected && !wasConnected) {
            ESP_LOGI(TAG, "BLE connected, will push data immediately");
            firstDataSent = false;
        }
        wasConnected = isConnected;

        // BLE 连接时: 立即推送第一包数据, 之后每2秒推送一次
        if (isConnected) {
            bool shouldPush = false;

            if (!firstDataSent) {
                // 连接后立即推送, 不等2秒
                // 但给 MTU 交换留一点时间 (延迟 500ms 再发第一包)
                if (now >= 500) {
                    shouldPush = true;
                    firstDataSent = true;
                }
            } else if ((now - lastDataPush) >= 2000) {
                shouldPush = true;
            }

            if (shouldPush) {
                lastDataPush = now;
                // 构建并发送数据 (数据由 task_hardware_poll 持续更新)
                const char* json = build_full_data_json();
                send_response(json);
            }
        } else {
            // 未连接时, 重置首包标记
            firstDataSent = false;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms 循环周期
    }
}

// ==================== FreeRTOS 任务函数 ====================

/**
 * task_hardware_poll - 硬件轮询任务
 *
 * 职责: 按固定周期轮询 PMBus 和 ADC 采样
 * 周期: 500ms
 * 优先级: 2
 */
static void task_hardware_poll(void *pvParameters)
{
    ESP_LOGI(TAG, "[HW_POLL] Task started");

    while (1) {
        // 扫描 PMBus 数据
        PMBus::scan();

        // ADC 采样
        ADCSampler::sample();

        // 500ms 轮询一次
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * task_ui_and_buttons - UI 与按键任务
 *
 * 职责: 处理按键输入, 更新 LCD 显示
 * 周期: 50ms (保证按键跟手)
 * 优先级: 3 (高于硬件轮询, 确保交互不卡顿)
 */
static void task_ui_and_buttons(void *pvParameters)
{
    ESP_LOGI(TAG, "[UI_BTN] Task started");

    uint64_t lastLcdUpdate = 0;

    // 如果显示屏初始化成功，显示启动画面（纯色填充）
    if (lcd_display_is_initialized()) {
        ESP_LOGI(TAG, "LCD display ready for updates");
    }

    while (1) {
        // 按键处理
        handle_buttons();

        // 每 2 秒更新一次显示屏（如果已初始化）
        uint64_t now = esp_timer_get_time() / 1000; // ms
        if (lcd_display_is_initialized() && (now - lastLcdUpdate) >= 2000) {
            lastLcdUpdate = now;
            // TODO: 后续可添加 LVGL 或自定义图形渲染
            // 当前仅保持驱动可用，显示内容后续实现
        }

        // 50ms 轮询一次按键，保证极致跟手
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ==================== 外设初始化 ====================

static void init_peripherals(void)
{
    // 初始化 NVS (BLE 需要)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 初始化 NimBLE
    nimble_port_init();

    // 初始化 ADC (外部电压采样 GPIO0)
    ret = ADCSampler::init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC init failed, external voltage reading disabled");
    }

    // 初始化 PMBus I2C
    ret = PMBus::init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "PMBus init failed, PSU data reading disabled");
    }

    // 初始化按键 GPIO (内部上拉, 低电平有效)
    gpio_config_t btnCfg = {};
    btnCfg.pin_bit_mask = (1ULL << BTN_UP) | (1ULL << BTN_DOWN) | (1ULL << BTN_OK);
    btnCfg.mode         = GPIO_MODE_INPUT;
    btnCfg.pull_up_en   = GPIO_PULLUP_ENABLE;
    btnCfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    btnCfg.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&btnCfg);

    // 初始化 ST7789 TFT 显示屏
    ret = lcd_display_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "LCD display init failed, continuing without display");
    } else {
        ESP_LOGI(TAG, "LCD display initialized (%dx%d)",
                 lcd_display_get_width(), lcd_display_get_height());

        // 初始化 LVGL 并绑定 ST7789 面板
        ret = lvgl_setup_init(lcd_display_get_io(),
                              lcd_display_get_panel());
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "LVGL init failed: %s", esp_err_to_name(ret));
        } else {
            ESP_LOGI(TAG, "LVGL initialized, running demo...");
            lvgl_demo_run();
        }
    }
}

// ==================== BLE 接收回调 ====================

static void on_ble_rx(const char* data, int len)
{
    ESP_LOGI(TAG, "BLE RX (%d bytes): %s", len, data);

    // ========== 分包累积: 将数据追加到缓冲区 ==========
    // BLE MTU 默认 23 字节, 有效载荷仅 20 字节
    // 完整的 JSON 命令可能超过 20 字节, 需要累积到收到 '\n' 才处理

    if (len <= 0) return;

    // 检查缓冲区溢出
    if (g_bleRxLen + len >= BLE_RX_BUF_SIZE) {
        ESP_LOGW(TAG, "RX buffer overflow, resetting");
        g_bleRxLen = 0;
        return;
    }

    // 追加数据
    memcpy(g_bleRxBuf + g_bleRxLen, data, len);
    g_bleRxLen += len;
    g_bleRxBuf[g_bleRxLen] = '\0';

    // 检查是否收到完整的行 (以 '\n' 结尾)
    // App 发送时会在 JSON 末尾添加 '\n'
    bool hasNewline = (g_bleRxBuf[g_bleRxLen - 1] == '\n');

    if (!hasNewline) {
        // 还没收到完整的一行, 等待更多数据
        ESP_LOGD(TAG, "RX: waiting for more data (current %d bytes)", g_bleRxLen);
        return;
    }

    // ========== 收到完整的一行, 开始处理 ==========
    // 移除末尾的 '\n' 和 '\r'
    while (g_bleRxLen > 0 &&
           (g_bleRxBuf[g_bleRxLen - 1] == '\n' || g_bleRxBuf[g_bleRxLen - 1] == '\r')) {
        g_bleRxBuf[--g_bleRxLen] = '\0';
    }

    if (g_bleRxLen == 0) {
        // 空行, 忽略
        return;
    }

    const char* line = g_bleRxBuf;
    ESP_LOGI(TAG, "RX complete line (%d bytes): %s", g_bleRxLen, line);

    // 解析 JSON 命令
    // 命令格式:
    //   {"cmd":"get_data"}
    //   {"cmd":"get_info"}
    //   {"cmd":"set","V_set":XX.XX}
    //   {"cmd":"set","I_set":XX.XX}
    //   {"cmd":"set","power":1|0}
    //   {"cmd":"clear_faults"}

    char cmd[32] = {};
    float val = 0.0f;
    int ival = 0;

    // 提取 "cmd" 字段值
    const char* cmdStart = strstr(line, "\"cmd\"");
    if (!cmdStart) {
        ESP_LOGW(TAG, "Invalid JSON: no 'cmd' field");
        g_bleRxLen = 0;
        return;
    }

    // 查找 cmd 值
    const char* valStart = strstr(cmdStart, ":");
    if (!valStart) { g_bleRxLen = 0; return; }
    valStart++; // 跳过 ':'

    // 跳过空白和引号
    while (*valStart == ' ' || *valStart == '\"') valStart++;

    const char* valEnd = valStart;
    while (*valEnd && *valEnd != '\"' && *valEnd != ' ' && *valEnd != ',') valEnd++;

    int cmdLen = valEnd - valStart;
    if (cmdLen > 31) cmdLen = 31;
    memcpy(cmd, valStart, cmdLen);
    cmd[cmdLen] = '\0';

    ESP_LOGI(TAG, "Parsed command: '%s'", cmd);

    // ---------- 处理命令 ----------

    if (strcmp(cmd, "get_data") == 0) {
        // 返回当前最新数据 (由 task_hardware_poll 持续更新)
        const char* json = build_full_data_json();
        send_response(json);
    }
    else if (strcmp(cmd, "get_info") == 0) {
        // 返回设备厂商信息
        PMBus::scan(); // 尝试扫描 (首次会读取 MFR)
        const char* json = PMBus::getInfoJson();
        send_response(json);
    }
    else if (strcmp(cmd, "set") == 0) {
        // 解析设置参数: V_set, I_set, power
        if (strstr(line, "\"V_set\"") || strstr(line, "\"V_set\":")) {
            const char* vp = strstr(line, "\"V_set\":");
            if (vp) {
                vp = strstr(vp, ":") + 1;
                val = strtof(vp, nullptr);
                if (val >= 0 && val <= PSU_VOLTAGE_MAX) {
                    PowerControl::setVoltage(val);
                    ESP_LOGI(TAG, "Voltage set to %.3fV", val);
                }
            }
        }

        if (strstr(line, "\"I_set\"") || strstr(line, "\"I_set\":")) {
            const char* ip = strstr(line, "\"I_set\":");
            if (ip) {
                ip = strstr(ip, ":") + 1;
                val = strtof(ip, nullptr);
                if (val >= 0 && val <= PSU_CURRENT_MAX) {
                    PowerControl::setCurrent(val);
                    ESP_LOGI(TAG, "Current set to %.3fA", val);
                }
            }
        }

        if (strstr(line, "\"power\"") || strstr(line, "\"power\":")) {
            const char* pp = strstr(line, "\"power\":");
            if (pp) {
                pp = strstr(pp, ":") + 1;
                ival = (int)strtof(pp, nullptr);
                if (ival == 1) {
                    PowerControl::powerOn();
                    ESP_LOGI(TAG, "Power ON");
                } else {
                    PowerControl::powerOff();
                    ESP_LOGI(TAG, "Power OFF");
                }
            }
        }

        // 确认响应: 返回当前状态
        // 注意: 使用 getSetVoltage/getSetCurrent 返回设定值, 而非实际遥测值
        // power 字段使用 isPoweredOn() 读取 PMBus V_out 实际状态
        snprintf(g_respBuf, sizeof(g_respBuf),
            "{\"ack\":\"ok\",\"power\":%d,\"V_set\":%.3f,\"I_set\":%.3f}",
            PowerControl::isPoweredOn() ? 1 : 0,
            PowerControl::getSetVoltage(),
            PowerControl::getSetCurrent());
        send_response(g_respBuf);
    }
    else if (strcmp(cmd, "clear_faults") == 0) {
        PMBus::clearFaults();
        send_response("{\"ack\":\"faults_cleared\"}");
    }
    else {
        ESP_LOGW(TAG, "Unknown command: '%s'", cmd);
        send_response("{\"error\":\"unknown_command\"}");
    }

    // 处理完毕, 清空缓冲区
    g_bleRxLen = 0;
}

// ==================== 辅助函数 ====================

static const char* build_full_data_json(void)
{
    // 构建包含 PMBus 数据 + ADC 外部电压 + 控制状态的综合 JSON
    snprintf(g_respBuf, sizeof(g_respBuf),
        "{"
        "\"V_out\":%.3f,\"I_out\":%.3f,"
        "\"V_in\":%.3f,\"I_in\":%.3f,"
        "\"W_out\":%.1f,\"W_in\":%.1f,"
        "\"E_out\":%.1f,\"E_in\":%.1f,"
        "\"V_ext\":%.3f,"
        "\"temperature\":[%.1f,%.1f,%.1f],"
        "\"fan_speed\":%.0f,"
        "\"power_on\":%d,"
        "\"V_set\":%.3f,\"I_set\":%.3f,"
        "\"device_online\":%s"
        "}",
        PMBus::V_out, PMBus::I_out,
        PMBus::V_in, PMBus::I_in,
        PMBus::W_out, PMBus::W_in,
        PMBus::E_out, PMBus::E_in,
        ADCSampler::getVoltage(),
        PMBus::temperature[0], PMBus::temperature[1], PMBus::temperature[2],
        PMBus::fanSpeed[0],
        PowerControl::isPoweredOn() ? 1 : 0,
        PowerControl::getSetVoltage(), PowerControl::getSetCurrent(),
        PMBus::isDeviceOnline() ? "true" : "false"
    );
    return g_respBuf;
}

static void send_response(const char* json)
{
    esp_err_t ret = ble_server_send(json);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to send BLE response");
    } else {
        ESP_LOGD(TAG, "BLE TX: %s", json);
    }
}

// ==================== 按键处理 ====================

static void handle_buttons(void)
{
    static uint64_t lastBtnTime = 0;
    uint64_t now = esp_timer_get_time() / 1000; // ms

    // 简易去抖: 200ms 间隔
    if ((now - lastBtnTime) < 200) return;

    bool btnUp   = (gpio_get_level(BTN_UP)   == 0); // 低电平有效
    bool btnDown = (gpio_get_level(BTN_DOWN) == 0);
    bool btnOk   = (gpio_get_level(BTN_OK)   == 0);

    if (!btnUp && !btnDown && !btnOk) return;

    lastBtnTime = now;

    float vSet = PowerControl::getSetVoltage();

    if (btnUp) {
        vSet += 0.1f;
        if (vSet > PSU_VOLTAGE_MAX) vSet = PSU_VOLTAGE_MAX;
        PowerControl::setVoltage(vSet);
        ESP_LOGI(TAG, "BTN_UP: V_set=%.1fV", vSet);
    }
    else if (btnDown) {
        vSet -= 0.1f;
        if (vSet < 0.0f) vSet = 0.0f;
        PowerControl::setVoltage(vSet);
        ESP_LOGI(TAG, "BTN_DOWN: V_set=%.1fV", vSet);
    }
    else if (btnOk) {
        // 切换电源开关
        if (PowerControl::isPoweredOn()) {
            PowerControl::powerOff();
            ESP_LOGI(TAG, "BTN_OK: Power OFF");
        } else {
            PowerControl::powerOn();
            ESP_LOGI(TAG, "BTN_OK: Power ON");
        }
    }

    // 如果 BLE 已连接, 推送状态更新
    if (ble_server_is_connected()) {
        const char* json = build_full_data_json();
        send_response(json);
    }
}
