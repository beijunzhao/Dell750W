/**
 * main.cpp - 台达/戴尔 750W 数控电源控制板主程序
 *
 * ESP32-C3 (ESP-IDF 框架)
 *
 * 架构: FreeRTOS 多任务并发
 * - task_hardware_poll: 硬件轮询 (PMBus + ADC), 50ms 周期, 优先级 2
 * - task_ui_and_buttons: UI 与按键处理, 50ms 周期, 优先级 3 (内部含 LVGL 刷新)
 * - app_main (BLE Task): BLE 状态监控与数据推送, 100ms 周期
 *
 * 安全开机时序 (setup):
 * 1. SW_CTRL = LOW (切断电源输出)
 * 2. V_PWM = 0 (0V DAC → 0V 输出), I_PWM = 0
 * 3. 延时 50ms 等待 RC 滤波器稳定
 * 4. 初始化其他外设 (I2C, ADC, BLE, 按键)
 *
 * 通信协议: JSON over BLE UART (Nordic UART Service)
 * 请求:
 * {"cmd":"get_data"}          → 返回遥测数据 + 校准参数
 * {"cmd":"get_info"}          → 返回设备厂商信息
 * {"cmd":"set","V_set":12.0}  → 设置电压
 * {"cmd":"set","I_set":50.0}  → 设置电流
 * {"cmd":"set","power":1}     → 开关电源 (1=开, 0=关)
 * {"cmd":"calibrate","V_mult":1.005,"V_offset":-0.05}  → 电压校准
 * {"cmd":"clear_faults"}      → 清除故障
 * 响应: {"V_out":12.0,...,"V_mult":1.0000,"V_offset":0.0000,"V_raw":12.1,...}
 */
#include "pin_map.h"
#include "power_control.h"
#include "adc_sampler.h"
#include "pmbus.h"
#include "ble_server.h"
#include "lcd_display.h"
#include "lvgl_setup.h"
#include "lvgl_ui.h"
#include "esp_lvgl_port.h"
#include "calibration.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

static const char* TAG = "Main";

/* ---- 长按 OK 进入校准模式的阈值 (ms) ---- */
#define LONG_PRESS_MS  2000

// ---------- 前向声明 ----------
extern "C" void nimble_host_task(void *param);
static void on_ble_rx(const char* data, int len);
static void init_peripherals(void);
static void handle_buttons(void);
static const char* build_full_data_json(void);
void send_response(const char* json);

// FreeRTOS 任务函数前向声明
static void task_hardware_poll(void *pvParameters);
static void task_ui_and_buttons(void *pvParameters);

// 全局 JSON 缓冲区 (BLE 发送用)
static char g_respBuf[1536];

// BLE RX 分包累积缓冲区 (MTU=23 时数据可能被分片)
// 最大支持 4KB 的 JSON 命令
#define BLE_RX_BUF_SIZE 4096
static char g_bleRxBuf[BLE_RX_BUF_SIZE];
static int g_bleRxLen = 0;

/* ===== 最终修复说明 ===========================================
 * 1. LVGL 刷新：在 task_ui_and_buttons 中周期性调用 lv_timer_handler()
 * 2. PMBus 数据竞争：提醒后续在 PMBus 类内部加锁
 * 3. set_range 重复剪裁：删除冗余的 setVoltage/setCurrent 调用
 * 4. 校准系数兼容：使用固定假数据 (1.0, 0.0)，因为实际校准已由6点插值引擎接管
 * 5. 后备按键模式电压上限：改用 PowerControl::getVMax()
 * ============================================================= */

// ---------- app_main 入口 ----------

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, " Dell/Delta 750W PSU Controller v1.0");
    ESP_LOGI(TAG, " Platform: ESP32-C3");
    ESP_LOGI(TAG, "========================================");

    /* ★ NVS 必须在 PowerControl::init() 之前初始化，否则读取 V_set/I_set/V_max/I_max 会失败 */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "FATAL: NVS init failed! System halted.");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // ========== 安全开机时序 (严禁修改) ==========

    // Step 0: 提前初始化校准模块（必须在 PowerControl::init() 之前，保证 PWM 恢复时校准数据可用）
    calibration_init();

    // 第一步: 锁定命门 - SW_CTRL 输出 LOW, V_PWM = 0 (0V), I_PWM = 0
    ESP_LOGI(TAG, "[BOOT SEQ] Step 1: Locking SW_CTRL LOW, setting PWM safe bias...");
    ret = PowerControl::init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "FATAL: PowerControl init failed! System halted.");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // 时序注释: 此时 V_PWM 输出 0% 占空比 (0V → 0V 物理输出), I_PWM 输出 0%
    // 硬件安全: 即使 MCU 在此后死机, 外部 RC 滤波器也会维持 V_DAC≈0V, I_DAC≈0V

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

    // 创建硬件轮询任务 (PMBus + ADC), 50ms 周期, 优先级 2
    BaseType_t taskCreated;
    taskCreated = xTaskCreate(
        task_hardware_poll,
        "hw_poll",
        4096,
        NULL,
        2,
        NULL
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
        4096,
        NULL,
        3,
        NULL
    );
    if (taskCreated != pdPASS) {
        ESP_LOGE(TAG, "FATAL: Failed to create UI/button task!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }
    ESP_LOGI(TAG, "UI/button task created (prio=3, stack=4096)");

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, " System ready. Advertising as '%s'", ble_server_get_device_name());
    ESP_LOGI(TAG, "========================================");

    // ========== 主循环 (BLE Task) ==========
    uint64_t lastDataPush = 0;
    bool firstDataSent = false;
    bool wasConnected = false;
    bool pushInfo = true;  /* 交替推送: true=设备信息, false=遥测 */

    while (1) {
        uint64_t now = esp_timer_get_time() / 1000;
        bool isConnected = ble_server_is_connected();

        if (isConnected && !wasConnected) {
            ESP_LOGI(TAG, "BLE connected, will push data immediately");
            firstDataSent = false;
        }
        wasConnected = isConnected;

        if (isConnected) {
            bool shouldPush = false;
            if (!firstDataSent) {
                if (now >= 500) {
                    shouldPush = true;
                    firstDataSent = true;
                }
            } else if ((now - lastDataPush) >= 2000) {
                shouldPush = true;
            }
            if (shouldPush) {
                lastDataPush = now;
                /* 交替推送设备信息和遥测数据，APP信息页自动更新 */
                if (pushInfo) {
                    PMBus::scan();                     // ★ 确保数据最新
                    const char* info = PMBus::getInfoJson();
                    send_response(info);
                } else {
                    const char* json = build_full_data_json();
                    send_response(json);
                }
                pushInfo = !pushInfo;
            }
        } else {
            firstDataSent = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// ==================== FreeRTOS 任务函数 ====================

static void task_hardware_poll(void *pvParameters)
{
    ESP_LOGI(TAG, "[HW_POLL] Task started");
    while (1) {
        PMBus::scan();                     // ★ 扫描更新全局静态数据
        ADCSampler::sample();              // 执行一次 ADC 采样

        /* 校准模式：更新 ADC 或 PMBus 电流读数 */
        if (calibration_is_active()) {
            if (calibration_get_type() == CALIB_TYPE_CURRENT) {
                calibration_update_pmbus(PMBus::I_out);
            } else {
                calibration_update_adc(ADCSampler::getRawAdc());
            }
        }

        /* 注意：PMBus::scan() 会将结果写入静态成员（如 PMBus::I_out）。
         * 如果 build_full_data_json() 被 BLE 主循环调用且同时本任务也在写，
         * 可能发生数据撕裂。建议后续在 PMBus 类内部使用互斥锁保护所有静态数据。 */
        vTaskDelay(pdMS_TO_TICKS(50));  /* 50ms 周期，ADC/PMBus 刷新率 20Hz */
    }
}

static void task_ui_and_buttons(void *pvParameters)
{
    ESP_LOGI(TAG, "[UI_BTN] Task started");
    while (1) {
        // 按键处理（内部含 lvgl_port_lock）
        handle_buttons();

        // ★ LVGL 定时器处理（驱动屏幕刷新、动画等），必须周期性调用
        if (lvgl_port_lock(10)) {
            lv_timer_handler();
            lvgl_port_unlock();
        }

        // 任务休眠，保持 UI 快速响应（50ms 足够，可改为更短如 10ms 使动画更流畅）
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// ==================== 外设初始化 ====================

static void init_peripherals(void)
{
    /* NVS 已在 app_main 顶部初始化，此处不再重复 */
    nimble_port_init();

    esp_err_t ret = ADCSampler::init();
    if (ret != ESP_OK) ESP_LOGW(TAG, "ADC init failed");

    ret = PMBus::init();
    if (ret != ESP_OK) ESP_LOGW(TAG, "PMBus init failed");

    gpio_config_t btnCfg = {};
    btnCfg.pin_bit_mask = (1ULL << BTN_UP) | (1ULL << BTN_DOWN) | (1ULL << BTN_OK);
    btnCfg.mode         = GPIO_MODE_INPUT;
    btnCfg.pull_up_en   = GPIO_PULLUP_ENABLE;
    btnCfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    btnCfg.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&btnCfg);

    /* calibration_init() 已在 app_main 的 Step 0 提前调用，此处不再重复 */
    ret = lcd_display_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "LCD display init failed");
    } else {
        ESP_LOGI(TAG, "LCD display initialized (%dx%d)",
                 lcd_display_get_width(), lcd_display_get_height());
        ret = lvgl_setup_init(lcd_display_get_io(), lcd_display_get_panel());
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "LVGL init failed: %s", esp_err_to_name(ret));
        } else {
            ESP_LOGI(TAG, "LVGL initialized, starting dashboard UI...");
            lvgl_port_lock(0);
            lvgl_ui_init();
            lvgl_port_unlock();
        }
    }
}

// ==================== BLE 接收回调 ====================

static void on_ble_rx(const char* data, int len)
{
    ESP_LOGI(TAG, "BLE RX (%d bytes): %s", len, data);

    if (len <= 0) return;
    if (g_bleRxLen + len >= BLE_RX_BUF_SIZE) {
        ESP_LOGW(TAG, "RX buffer overflow, resetting");
        g_bleRxLen = 0;
        return;
    }
    memcpy(g_bleRxBuf + g_bleRxLen, data, len);
    g_bleRxLen += len;
    g_bleRxBuf[g_bleRxLen] = '\0';

    bool hasNewline = (g_bleRxBuf[g_bleRxLen - 1] == '\n');
    if (!hasNewline) return;

    while (g_bleRxLen > 0 &&
           (g_bleRxBuf[g_bleRxLen - 1] == '\n' || g_bleRxBuf[g_bleRxLen - 1] == '\r')) {
        g_bleRxBuf[--g_bleRxLen] = '\0';
    }
    if (g_bleRxLen == 0) return;

    const char* line = g_bleRxBuf;
    ESP_LOGI(TAG, "RX complete line (%d bytes): %s", g_bleRxLen, line);

    char cmd[32] = {};
    float val = 0.0f;
    int ival = 0;

    const char* cmdStart = strstr(line, "\"cmd\"");
    if (!cmdStart) {
        ESP_LOGW(TAG, "Invalid JSON: no 'cmd' field");
        g_bleRxLen = 0;
        return;
    }
    const char* valStart = strstr(cmdStart, ":");
    if (!valStart) { g_bleRxLen = 0; return; }
    valStart++;
    while (*valStart == ' ' || *valStart == '\"') valStart++;
    const char* valEnd = valStart;
    while (*valEnd && *valEnd != '\"' && *valEnd != ' ' && *valEnd != ',') valEnd++;
    int cmdLen = valEnd - valStart;
    if (cmdLen > 31) cmdLen = 31;
    memcpy(cmd, valStart, cmdLen);
    cmd[cmdLen] = '\0';
    ESP_LOGI(TAG, "Parsed command: '%s'", cmd);

    if (strcmp(cmd, "get_data") == 0) {
        const char* json = build_full_data_json();
        send_response(json);
    }
    else if (strcmp(cmd, "get_info") == 0) {
        PMBus::scan();                     // 保证数据最新
        const char* json = PMBus::getInfoJson();
        send_response(json);
    }
    else if (strcmp(cmd, "set") == 0) {
        if (strstr(line, "\"V_set\"") || strstr(line, "\"V_set\":")) {
            const char* vp = strstr(line, "\"V_set\":");
            if (vp) { vp = strstr(vp, ":") + 1; val = strtof(vp, nullptr);
                if (val >= 0 && val <= PowerControl::getVMax()) PowerControl::setVoltage(val);
                else ESP_LOGW(TAG, "V_set %.3f out of range", val); }
        }
        if (strstr(line, "\"I_set\"") || strstr(line, "\"I_set\":")) {
            const char* ip = strstr(line, "\"I_set\":");
            if (ip) { ip = strstr(ip, ":") + 1; val = strtof(ip, nullptr);
                if (val >= 0 && val <= PowerControl::getIMax()) PowerControl::setCurrent(val);
                else ESP_LOGW(TAG, "I_set %.3f out of range", val); }
        }
        if (strstr(line, "\"power\"") || strstr(line, "\"power\":")) {
            const char* pp = strstr(line, "\"power\":");
            if (pp) { pp = strstr(pp, ":") + 1; ival = (int)strtof(pp, nullptr);
                if (ival == 1) PowerControl::powerOn(); else PowerControl::powerOff(); }
        }
        
        // ★ 加上这一行：在用户通过 APP 设定完参数后，统一保存一次 NVS
        PowerControl::saveToNVS();
        
        snprintf(g_respBuf, sizeof(g_respBuf),
            "{\"ack\":\"ok\",\"power\":%d,\"V_set\":%.3f,\"I_set\":%.3f}",
            PowerControl::isPoweredOn() ? 1 : 0,
            PowerControl::getSetVoltage(), PowerControl::getSetCurrent());
        send_response(g_respBuf);
    }
    else if (strcmp(cmd, "clear_faults") == 0) {
        PMBus::clearFaults();
        send_response("{\"ack\":\"faults_cleared\"}");
    }
    else if (strcmp(cmd, "calibrate") == 0) {
        send_response("{\"error\":\"calibrate deprecated, use cal_mode\"}");
    }
    else if (strcmp(cmd, "cal_mode") == 0) {
        const char* ep = strstr(line, "\"enter\":");
        if (ep) { ep = strstr(ep, ":") + 1; ival = (int)strtof(ep, nullptr);
            if (ival == 1) { if (!calibration_is_active()) calibration_start_v(); send_response("{\"ack\":\"cal_mode_entered\"}"); }
            else { if (calibration_is_active()) calibration_stop(); send_response("{\"ack\":\"cal_mode_exited\"}"); } }
    }
    else if (strcmp(cmd, "cal_pwm_adjust") == 0) {
        if (!calibration_is_active()) send_response("{\"error\":\"not_in_cal_mode\"}");
        else {
            const char* dp = strstr(line, "\"dir\":");
            if (dp) { dp = strstr(dp, ":") + 1; ival = (int)strtof(dp, nullptr);
                calibration_handle_button(ival > 0, ival < 0, false);
                send_response("{\"ack\":\"pwm_adjusted\"}"); }
        }
    }
    else if (strcmp(cmd, "cal_confirm") == 0) {
        if (!calibration_is_active()) send_response("{\"error\":\"not_in_cal_mode\"}");
        else { calibration_handle_button(false, false, true); send_response("{\"ack\":\"cal_confirmed\"}"); }
    }
    else if (strcmp(cmd, "set_i_cal_table") == 0) {
        PMBus::i_calib_point_t points[PMBus::I_CALIB_POINTS];
        for (int i = 0; i < PMBus::I_CALIB_POINTS; i++) {
            points[i].target = PMBus::I_CALIB_TARGETS[i];
            points[i].pwm_val = 0.0f; points[i].raw_val = 0.0f;
        }
        const char* pts = strstr(line, "\"points\"");
        if (pts) { pts = strstr(pts, "["); if (pts) { pts++;
            for (int i = 0; i < PMBus::I_CALIB_POINTS; i++) {
                const char* rp = strstr(pts, "\"r\":");
                if (rp) { rp = strstr(rp, ":") + 1; points[i].raw_val = strtof(rp, nullptr); }
                const char* vp = strstr(pts, "\"v\":");
                if (vp) { vp = strstr(vp, ":") + 1; points[i].target = strtof(vp, nullptr); }
                const char* pp = strstr(pts, "\"p\":");
                if (pp) { pp = strstr(pp, ":") + 1; points[i].pwm_val = strtof(pp, nullptr); }
                pts = strstr(pts, "}"); if (pts) pts++; } } }
        PMBus::setCurrentCalTable(points);
        char resp[512]; int pos = snprintf(resp, sizeof(resp), "{\"ack\":\"i_cal_table_ok\",\"i_cal_points\":[");
        for (int i = 0; i < PMBus::I_CALIB_POINTS; i++)
            pos += snprintf(resp+pos, sizeof(resp)-pos, "%s{\"r\":%.3f,\"v\":%.1f,\"p\":%.1f}", (i>0?",":""), points[i].raw_val, points[i].target, points[i].pwm_val);
        snprintf(resp+pos, sizeof(resp)-pos, "]}"); send_response(resp);
    }
    else if (strcmp(cmd, "set_range") == 0) {
        const char* vp = strstr(line, "\"v_max\":");
        if (vp) { vp = strstr(vp, ":") + 1; float nv=strtof(vp, nullptr); PowerControl::setVMax(nv); /* 内部已自动裁剪当前电压 */ }
        const char* ip = strstr(line, "\"i_max\":");
        if (ip) { ip = strstr(ip, ":") + 1; float ni=strtof(ip, nullptr); PowerControl::setIMax(ni); /* 内部已自动裁剪当前电流 */ }
        snprintf(g_respBuf, sizeof(g_respBuf), "{\"ack\":\"range_ok\",\"V_max\":%.1f,\"I_max\":%.1f}",
                 PowerControl::getVMax(), PowerControl::getIMax());
        send_response(g_respBuf);
    }
    else {
        ESP_LOGW(TAG, "Unknown command: '%s'", cmd);
        send_response("{\"error\":\"unknown_command\"}");
    }
    g_bleRxLen = 0;
}

// ==================== 辅助函数 ====================

static const char* build_full_data_json(void)
{
    int cal_step = 0, cal_pwm = 0, cal_adc = 0;
    float cal_target = 0.0f;
    bool cal_active = calibration_is_active();
    if (cal_active) {
        int step = calibration_get_current_step();
        cal_step = step + 1; if (cal_step < 1) cal_step = 1;
        cal_pwm = calibration_get_pwm();
        cal_adc = ADCSampler::getRawAdc();
        // 目标值：根据校准类型选择对应的数组
        if (calibration_get_type() == CALIB_TYPE_VOLTAGE)
            cal_target = g_calib_v_targets[step];
        else
            cal_target = g_calib_i_targets[step];
    }

    // 兼容 APP 的 JSON 格式，实际校准已由底层的 6 点插值引擎接管
    float v_mult = 1.0f, v_offset = 0.0f;
    float i_mult = 1.0f, i_offset = 0.0f;

    snprintf(g_respBuf, sizeof(g_respBuf),
        "{\"V_out\":%.3f,\"I_out\":%.3f,\"V_in\":%.3f,\"I_in\":%.3f,"
        "\"W_out\":%.1f,\"W_in\":%.1f,\"E_out\":%.1f,\"E_in\":%.1f,"
        "\"V_ext\":%.3f,\"temperature\":[%.1f,%.1f,%.1f],\"fan_speed\":%.0f,"
        "\"power_on\":%d,\"V_set\":%.3f,\"I_set\":%.3f,"
        "\"V_mult\":%.4f,\"V_offset\":%.4f,\"I_mult\":%.4f,\"I_offset\":%.4f,\"V_raw\":%.3f,"
        "\"device_online\":%s,"
        "\"cal_mode\":%d,\"cal_step\":%d,\"cal_target\":%.2f,\"cal_adc\":%d,\"cal_pwm\":%d,"
        "\"V_max\":%.1f,\"I_max\":%.1f}",
        ADCSampler::getVoltage(), PMBus::I_out,
        PMBus::V_in, PMBus::I_in,
        PMBus::W_out, PMBus::W_in, PMBus::E_out, PMBus::E_in,
        PMBus::V_out,
        PMBus::temperature[0], PMBus::temperature[1], PMBus::temperature[2],
        PMBus::fanSpeed[0],
        PowerControl::isPoweredOn() ? 1 : 0,
        PowerControl::getSetVoltage(), PowerControl::getSetCurrent(),
        v_mult, v_offset, i_mult, i_offset,
        ADCSampler::getRawVoltage(),
        PMBus::isDeviceOnline() ? "true" : "false",
        cal_active ? 1 : 0, cal_step, cal_target, cal_adc, cal_pwm,
        PowerControl::getVMax(), PowerControl::getIMax());
    return g_respBuf;
}

void send_response(const char* json)
{
    esp_err_t ret = ble_server_send(json);
    if (ret != ESP_OK) ESP_LOGW(TAG, "Failed to send BLE response");
    else ESP_LOGD(TAG, "BLE TX: %s", json);
}

// ==================== 按键处理 ====================

uint64_t g_lastUpTime = 0;
uint64_t g_lastDownTime = 0;

void lvgl_reset_debounce(void) { g_lastUpTime = 0; g_lastDownTime = 0; }

static void handle_buttons(void)
{
    static uint64_t okPressStart = 0;
    static bool     okWasPressed = false;
    static bool     btnOkLast = false;
    static bool     longPressJustFired = false;
    uint64_t now = esp_timer_get_time() / 1000;

    bool btnUp   = (gpio_get_level(BTN_UP)   == 0);
    bool btnDown = (gpio_get_level(BTN_DOWN) == 0);
    bool btnOk   = (gpio_get_level(BTN_OK)   == 0);

    // 校准模式
    if (calibration_is_active()) {
        // 长按 OK 检测: 停止校准并返回菜单
        if (btnOk) {
            if (!okWasPressed) { okWasPressed = true; okPressStart = now; }
            else if ((now - okPressStart) >= LONG_PRESS_MS) {
                okWasPressed = false; okPressStart = 0;
                calibration_stop();
                return;
            }
        } else {
            okWasPressed = false;
        }
        static uint64_t calLast = 0;
        if ((now - calLast) < 100) return;
        if (!btnUp && !btnDown && !btnOk) return;
        calLast = now;
        calibration_handle_button(btnUp, btnDown, btnOk);
        return;
    }

    // 非 LVGL 模式 (后备)
    if (!lvgl_ui_is_ready()) {
        if (btnOk) {
            if (!okWasPressed) { okWasPressed = true; okPressStart = now; }
            if ((now - okPressStart) >= LONG_PRESS_MS) { okWasPressed = false; calibration_start_v(); return; }
        } else { okWasPressed = false; }
        static uint64_t lastFallbackTime = 0;
        if ((now - lastFallbackTime) < 200) return;
        if (!btnUp && !btnDown && !btnOk) return;
        lastFallbackTime = now;

        // ★ 修正：使用当前量程上限 PowerControl::getVMax()，而非硬编码的全局最大值
        float maxV = PowerControl::getVMax();
        if (btnUp) {
            float v = PowerControl::getSetVoltage() + 0.1f;
            if (v > maxV) v = maxV;
            PowerControl::setVoltage(v);
        } else if (btnDown) {
            float v = PowerControl::getSetVoltage() - 0.1f;
            if (v < 0) v = 0;
            PowerControl::setVoltage(v);
        } else if (btnOk) {
            if (PowerControl::isPoweredOn()) PowerControl::powerOff();
            else PowerControl::powerOn();
        }
        return;
    }

    // ===== LVGL 正常模式 =====

    // 长按 OK 检测 (2秒)
    if (btnOk) {
        if (!okWasPressed && !longPressJustFired) { okWasPressed = true; okPressStart = now; }
        if (okWasPressed && (now - okPressStart) >= LONG_PRESS_MS) {
            okWasPressed = false; okPressStart = 0;
            longPressJustFired = true;
            if (lvgl_port_lock(200)) {
                lvgl_ui_handle_key(LVGL_KEY_OK_LONG);
                lvgl_port_unlock();
            }
            return;
        }
    } else {
        if (longPressJustFired) {
            longPressJustFired = false;
        } else if (btnOkLast && okWasPressed) {
            okWasPressed = false; okPressStart = 0;
            if (lvgl_port_lock(200)) {
                lvgl_ui_handle_key(LVGL_KEY_OK);
                lvgl_port_unlock();
            }
        }
        if (!btnOk) { okWasPressed = false; okPressStart = 0; }
    }
    btnOkLast = btnOk;

    // UP 键 → LVGL_KEY_UP
    if (btnUp && (now - g_lastUpTime) >= 200) {
        g_lastUpTime = now;
        if (lvgl_port_lock(200)) {
            lvgl_ui_handle_key(LVGL_KEY_UP);
            lvgl_port_unlock();
        }
    }

    // DOWN 键 → LVGL_KEY_DOWN
    if (btnDown && (now - g_lastDownTime) >= 200) {
        g_lastDownTime = now;
        if (lvgl_port_lock(200)) {
            lvgl_ui_handle_key(LVGL_KEY_DOWN);
            lvgl_port_unlock();
        }
    }
}