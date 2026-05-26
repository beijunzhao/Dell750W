/**
 * calibration.cpp - ADC/PWM 校准状态机实现 (BLE 控制版)
 *
 * 校准流程 (6 步):
 *   1. App 发送 cal_mode 命令进入校准模式
 *   2. App 发送 cal_pwm_adjust(dir:1/-1) 调节 PWM
 *   3. 用户用万用表测量输出, 对齐目标电压后 App 发送 cal_confirm
 *   4. ESP32 记录当前 PWM 和 ADC 值, 通过 BLE 推送结果
 *   5. 重复直到 6 个点全部完成
 *   6. 完成时自动计算校准参数并保存到 NVS
 *
 * ===== 状态机 =====
 * IDLE → (calibration_start) → ADJUSTING(step=0)
 *   → UP/DOWN 调节 PWM → OK 确认 → 保存数据 → step++
 *   → step==6 → COMPLETE → 计算参数 → 保存 NVS → 退出
 *
 * ===== PWM 调节逻辑 (正向) =====
 * 初始 PWM = 从目标电压正算
 * UP:   PWM += PWM_STEP (电压升高)
 * DOWN: PWM -= PWM_STEP (电压降低)
 * 钳位: [0, PWM_MAX_DUTY]
 */
#include "calibration.h"
#include "power_control.h"
#include "adc_sampler.h"
#include "pin_map.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "Calib";

/* ---- NVS 命名空间 ---- */
#define NVS_NS_CALIB   "calib_data"

/* ---- PWM 调节步进 ---- */
#define PWM_STEP  20

/* ---- 目标电压数组 ---- */
const float g_calib_targets[CALIB_POINTS] = {0.0f, 1.0f, 2.5f, 5.0f, 10.0f, 12.0f};

/* ========== 静态变量 ========== */

static calib_state_t s_state = CALIB_STATE_IDLE;
static int           s_step  = 0;           /**< 当前步骤 0~5 */
static int           s_pwm   = 0;           /**< 当前 PWM 占空比 (用户调节中) */
static int           s_adc   = 0;           /**< 最新 ADC 原始读数 */
static calib_data_t  s_data[CALIB_POINTS];  /**< 6 点校准数据 */

/* ========== 前向声明 ========== */
static void _apply_pwm(int pwm);
static void _on_step_confirm(void);
static void _on_complete(void);
static void _save_to_nvs(void);
static bool _load_from_nvs(void);

/* ========== 公开 API ========== */

void calibration_init(void)
{
    // 尝试从 NVS 加载之前保存的校准数据 (仅用于显示, 不自动进入校准)
    _load_from_nvs();
    ESP_LOGI(TAG, "Calibration module initialized");
}

void calibration_start(void)
{
    if (s_state != CALIB_STATE_IDLE) {
        ESP_LOGW(TAG, "Already in calibration, ignoring start");
        return;
    }

    ESP_LOGI(TAG, "===== Calibration START =====");

    s_step = 0;
    s_state = CALIB_STATE_ADJUSTING;

    // 初始 PWM: 从目标电压正算 (正向逻辑)
    // V_DAC = (V_target / 12.0) * 3.0
    // PWM = (V_DAC / 3.3) * 2047
    float v_target = g_calib_targets[0];
    float v_dac = (v_target / PSU_VOLTAGE_MAX) * V_DAC_MAX;
    s_pwm = (int)((v_dac / MCU_VDD) * PWM_MAX_DUTY);
    if (s_pwm < 0)   s_pwm = 0;
    if (s_pwm > PWM_MAX_DUTY) s_pwm = PWM_MAX_DUTY;

    s_adc = ADCSampler::getRawAdc();

    // 清空校准数据
    for (int i = 0; i < CALIB_POINTS; i++) {
        s_data[i].target  = g_calib_targets[i];
        s_data[i].pwm_val = 0;
        s_data[i].adc_val = 0;
    }

    // 应用初始 PWM
    _apply_pwm(s_pwm);

    ESP_LOGI(TAG, "Step %d/%d: target=%.2fV, initial PWM=%d",
             s_step + 1, CALIB_POINTS, g_calib_targets[s_step], s_pwm);
}

void calibration_stop(void)
{
    if (s_state == CALIB_STATE_IDLE) return;

    ESP_LOGI(TAG, "===== Calibration STOP =====");
    s_state = CALIB_STATE_IDLE;
    s_step  = 0;

    ESP_LOGI(TAG, "Calibration stopped, returned to idle");
}

bool calibration_is_active(void)
{
    return (s_state != CALIB_STATE_IDLE);
}

void calibration_handle_button(bool btn_up, bool btn_down, bool btn_ok)
{
    if (s_state != CALIB_STATE_ADJUSTING) return;

    if (btn_up) {
        // UP: PWM 增加 (电压升高, 正向逻辑)
        s_pwm += PWM_STEP;
        if (s_pwm > PWM_MAX_DUTY) s_pwm = PWM_MAX_DUTY;
        _apply_pwm(s_pwm);
        ESP_LOGI(TAG, "Step %d: PWM UP -> %d", s_step + 1, s_pwm);
    }
    else if (btn_down) {
        // DOWN: PWM 减少 (电压降低)
        s_pwm -= PWM_STEP;
        if (s_pwm < 0) s_pwm = 0;
        _apply_pwm(s_pwm);
        ESP_LOGI(TAG, "Step %d: PWM DOWN -> %d", s_step + 1, s_pwm);
    }
    else if (btn_ok) {
        // OK: 确认当前点
        _on_step_confirm();
    }
}

void calibration_update_adc(int raw_adc)
{
    if (s_state != CALIB_STATE_ADJUSTING) return;

    s_adc = raw_adc;
}

const calib_data_t* calibration_get_data(void)
{
    return s_data;
}

int calibration_get_current_step(void)
{
    if (s_state == CALIB_STATE_IDLE) return -1;
    return s_step;
}

/* ========== 私有函数 ========== */

/**
 * 应用 PWM 到硬件
 * 注意: 电压控制是正向逻辑, 这里直接设置 PWM 占空比
 * 不经过 PowerControl::setVoltage() 的数学转换
 */
static void _apply_pwm(int pwm)
{
    // 直接设置 LEDC 通道 0 (V_PWM) 的占空比
    PowerControl::setPwmDuty(LEDC_CHANNEL_0, (uint32_t)pwm);
}

/**
 * 确认当前步骤, 保存数据并进入下一步
 */
// 前向声明 BLE 推送函数 (在 main.cpp 中实现)
extern void send_response(const char* json);

static void _on_step_confirm(void)
{
    // 保存当前点的数据
    s_data[s_step].pwm_val = s_pwm;
    s_data[s_step].adc_val = s_adc;

    ESP_LOGI(TAG, "Step %d confirmed: target=%.2fV, PWM=%d, ADC=%d",
             s_step + 1,
             g_calib_targets[s_step], s_pwm, s_adc);

    // 通过 BLE 推送校准结果到 App
    char buf[256];
    snprintf(buf, sizeof(buf),
        "{\"cal_result_target\":%.2f,\"cal_result_adc\":%d,\"cal_result_pwm\":%d}",
        g_calib_targets[s_step], s_adc, s_pwm);
    send_response(buf);

    s_step++;

    if (s_step >= CALIB_POINTS) {
        // 所有 6 点完成
        _on_complete();
    } else {
        // 切换到下一个目标电压
        float v_target = g_calib_targets[s_step];

        // 计算新 PWM: 从目标电压正算 (正向逻辑)
        float v_dac = (v_target / PSU_VOLTAGE_MAX) * V_DAC_MAX;
        s_pwm = (int)((v_dac / MCU_VDD) * PWM_MAX_DUTY);
        if (s_pwm < 0)   s_pwm = 0;
        if (s_pwm > PWM_MAX_DUTY) s_pwm = PWM_MAX_DUTY;

        _apply_pwm(s_pwm);

        ESP_LOGI(TAG, "Step %d/%d: target=%.2fV, initial PWM=%d",
                 s_step + 1, CALIB_POINTS, v_target, s_pwm);
    }
}

/**
 * 全部 6 点完成: 计算校准参数并保存到 NVS
 */
static void _on_complete(void)
{
    ESP_LOGI(TAG, "===== Calibration COMPLETE =====");

    s_state = CALIB_STATE_COMPLETE;

    // 打印所有校准数据
    for (int i = 0; i < CALIB_POINTS; i++) {
        ESP_LOGI(TAG, "  Point %d: target=%.2fV, PWM=%d, ADC=%d",
                 i + 1, s_data[i].target, s_data[i].pwm_val, s_data[i].adc_val);
    }

    // 保存到 NVS
    _save_to_nvs();

    // 计算线性校准参数 (mult 和 offset)
    // 使用第一个点 (0V) 和最后一个点 (12V) 做两点校准
    // V_real = V_raw * mult + offset
    // 其中 V_raw = adc_val / 4095 * 3.3 * ADC_DIVIDER_RATIO
    if (s_data[0].adc_val != 0 && s_data[CALIB_POINTS - 1].adc_val != 0) {
        float v_raw_0  = (float)s_data[0].adc_val / 4095.0f * MCU_VDD * ADC_DIVIDER_RATIO;
        float v_raw_12 = (float)s_data[CALIB_POINTS - 1].adc_val / 4095.0f * MCU_VDD * ADC_DIVIDER_RATIO;

        float target_0  = s_data[0].target;
        float target_12 = s_data[CALIB_POINTS - 1].target;

        // V_cal = V_raw * mult + offset
        // mult = (V_target_12 - V_target_0) / (V_raw_12 - V_raw_0)
        // offset = V_target_0 - V_raw_0 * mult
        float mult = (target_12 - target_0) / (v_raw_12 - v_raw_0);
        float offset = target_0 - v_raw_0 * mult;

        ESP_LOGI(TAG, "Calculated cal: mult=%.4f offset=%.4f", mult, offset);

        // 应用到 ADCSampler
        ADCSampler::calibrate(mult, offset);
    } else {
        ESP_LOGW(TAG, "ADC values invalid, skipping calibration calculation");
    }

    // 自动退出校准
    calibration_stop();
}

/* ========== NVS 持久化 ========== */

static void _save_to_nvs(void)
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NS_CALIB, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = nvs_set_blob(handle, "points", s_data, sizeof(s_data));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS write failed: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return;
    }

    ret = nvs_commit(handle);
    nvs_close(handle);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration data saved to NVS (%d bytes)", (int)sizeof(s_data));
    } else {
        ESP_LOGE(TAG, "NVS commit failed: %s", esp_err_to_name(ret));
    }
}

static bool _load_from_nvs(void)
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NS_CALIB, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "No calibration data in NVS");
        return false;
    }

    size_t sz = sizeof(s_data);
    ret = nvs_get_blob(handle, "points", s_data, &sz);
    nvs_close(handle);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Loaded %d calibration points from NVS", CALIB_POINTS);
        for (int i = 0; i < CALIB_POINTS; i++) {
            ESP_LOGI(TAG, "  [%d] target=%.2fV PWM=%d ADC=%d",
                     i, s_data[i].target, s_data[i].pwm_val, s_data[i].adc_val);
        }
        return true;
    }

    ESP_LOGD(TAG, "No calibration data found");
    return false;
}
