/**
 * calibration.cpp - 6点校准状态机 + 分段线性插值
 *
 * 电压校准: V_PWM(LEDC_CH0) + ADC → 6点分段线性插值
 * 电流校准: I_PWM(LEDC_CH1) + PMBus I_out → 6点查表修正映射
 *
 * PWM初始值动态倒算:
 *   v_pwm = (v_target / getVMax()) * PWM_MAX_DUTY
 *   i_pwm = (i_target / getIMax()) * PWM_MAX_DUTY
 *   当 getVMax/getIMax <= 0.1 时回退到默认物理量程
 */
#include "calibration.h"
#include "power_control.h"
#include "adc_sampler.h"
#include "pmbus.h"
#include "pin_map.h"
#include <cmath>
#include "lvgl_ui.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "Calib";
#define NVS_NS_CALIB   "calib_data"
#define PWM_STEP  20

/* ---- 默认目标值（当量程异常时回退） ---- */
const float g_calib_v_targets[CALIB_POINTS] = {0.0f, 1.0f, 2.5f, 5.0f, 10.0f, 12.0f};
const float g_calib_i_targets[CALIB_POINTS] = {0.0f, 10.0f, 20.0f, 30.0f, 50.0f, 62.5f};

/* ---- 静态变量 ---- */
static calib_state_t  s_state = CALIB_STATE_IDLE;
static calib_type_t   s_type  = CALIB_TYPE_VOLTAGE;
static int            s_step  = 0;
static int            s_pwm   = 0;
static int            s_adc   = 0;
static float          s_pmbus_i = 0.0f;
static calib_point_t  s_data[CALIB_POINTS];  /* 6点校准数据 */

static void _apply_pwm(int pwm);
static void _on_step_confirm(void);
static void _on_complete(void);
static void _save_to_nvs(void);
static bool _load_from_nvs(void);

/* ========== 公开 API ========== */

void calibration_init(void)
{
    _load_from_nvs();
    ESP_LOGI(TAG, "Calibration module initialized");
}

/** 计算 PWM 初始值（动态量程适配 + 防呆） */
static int _calc_init_pwm(float target, bool is_voltage)
{
    float range_max;
    if (is_voltage) {
        range_max = PowerControl::getVMax();
        if (range_max <= 0.1f) range_max = PSU_VOLTAGE_MAX;      /* 12.0V */
    } else {
        range_max = PowerControl::getIMax();
        if (range_max <= 0.1f) range_max = PSU_CURRENT_MAX;      /* 62.5A */
    }
    float v_dac = (target / range_max) * V_DAC_MAX;
    int pwm = (int)((v_dac / MCU_VDD) * PWM_MAX_DUTY);
    if (pwm < 0) pwm = 0;
    if (pwm > PWM_MAX_DUTY) pwm = PWM_MAX_DUTY;
    return pwm;
}

void calibration_start_v(void)
{
    if (s_state != CALIB_STATE_IDLE) return;
    ESP_LOGI(TAG, "===== Voltage Calibration START =====");
    s_type = CALIB_TYPE_VOLTAGE;
    s_step = 0;
    s_state = CALIB_STATE_ADJUSTING;
    lvgl_show_calibration();

    s_pwm = _calc_init_pwm(g_calib_v_targets[0], true);
    s_adc = ADCSampler::getRawAdc();

    for (int i = 0; i < CALIB_POINTS; i++) {
        s_data[i].target_v    = g_calib_v_targets[i];
        s_data[i].target_i    = 0;
        s_data[i].adc_raw     = 0;
        s_data[i].pmbus_i_raw = 0.0f;
        s_data[i].pwm_val     = 0;
    }
    _apply_pwm(s_pwm);
    ESP_LOGI(TAG, "V-Step %d/%d: target=%.2fV, PWM=%d (VMax=%.1fV)",
             s_step+1, CALIB_POINTS, g_calib_v_targets[s_step], s_pwm,
             PowerControl::getVMax());
}

void calibration_start_i(void)
{
    if (s_state != CALIB_STATE_IDLE) return;
    ESP_LOGI(TAG, "===== Current Calibration START =====");
    s_type = CALIB_TYPE_CURRENT;
    s_step = 0;
    s_state = CALIB_STATE_ADJUSTING;
    lvgl_show_current_calibration();

    s_pwm = _calc_init_pwm(g_calib_i_targets[0], false);
    s_pmbus_i = PMBus::I_out;

    for (int i = 0; i < CALIB_POINTS; i++) {
        s_data[i].target_v    = 0;
        s_data[i].target_i    = g_calib_i_targets[i];
        s_data[i].adc_raw     = 0;
        s_data[i].pmbus_i_raw = 0.0f;
        s_data[i].pwm_val     = 0;
    }
    _apply_pwm(s_pwm);
    ESP_LOGI(TAG, "I-Step %d/%d: target=%.1fA, PWM=%d (IMax=%.1fA)",
             s_step+1, CALIB_POINTS, g_calib_i_targets[s_step], s_pwm,
             PowerControl::getIMax());
}

void calibration_stop(void)
{
    if (s_state == CALIB_STATE_IDLE) return;
    ESP_LOGI(TAG, "===== Calibration STOP =====");
    s_state = CALIB_STATE_IDLE;
    s_step = 0;
    lvgl_hide_calibration();
}

bool calibration_is_active(void) { return s_state != CALIB_STATE_IDLE; }
calib_type_t calibration_get_type(void) { return s_type; }

void calibration_handle_button(bool btn_up, bool btn_down, bool btn_ok)
{
    if (s_state != CALIB_STATE_ADJUSTING) return;
    static bool last_ok = false;
    bool ok_rising = btn_ok && !last_ok;
    last_ok = btn_ok;

    if (btn_up) {
        s_pwm += PWM_STEP;
        if (s_pwm > PWM_MAX_DUTY) s_pwm = PWM_MAX_DUTY;
        _apply_pwm(s_pwm);
    } else if (btn_down) {
        s_pwm -= PWM_STEP;
        if (s_pwm < 0) s_pwm = 0;
        _apply_pwm(s_pwm);
    } else if (ok_rising) {
        _on_step_confirm();
    }
}

void calibration_update_adc(int raw_adc)
{
    if (s_state != CALIB_STATE_ADJUSTING || s_type != CALIB_TYPE_VOLTAGE) return;
    s_adc = raw_adc;
}

void calibration_update_pmbus(float current)
{
    if (s_state != CALIB_STATE_ADJUSTING || s_type != CALIB_TYPE_CURRENT) return;
    s_pmbus_i = current;
}

const calib_point_t* calibration_get_data(void) { return s_data; }
int calibration_get_current_step(void) { return (s_state == CALIB_STATE_IDLE) ? -1 : s_step; }
int calibration_get_pwm(void) { return s_pwm; }
float calibration_get_pmbus_value(void) { return s_pmbus_i; }

/* ========== 私有函数 ========== */

static void _apply_pwm(int pwm)
{
    ledc_channel_t ch = (s_type == CALIB_TYPE_VOLTAGE) ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1;
    PowerControl::setPwmDuty(ch, (uint32_t)pwm);
}

extern void send_response(const char* json);

static void _on_step_confirm(void)
{
    if (s_type == CALIB_TYPE_VOLTAGE) {
        s_data[s_step].adc_raw = s_adc;
    } else {
        s_data[s_step].pmbus_i_raw = s_pmbus_i;
    }
    s_data[s_step].pwm_val = s_pwm;

    ESP_LOGI(TAG, "Step %d confirmed: %s target=%.2f, PWM=%d, raw=%d/%.4f",
             s_step + 1,
             (s_type == CALIB_TYPE_VOLTAGE) ? "V" : "I",
             (s_type == CALIB_TYPE_VOLTAGE) ? s_data[s_step].target_v : s_data[s_step].target_i,
             s_pwm,
             (s_type == CALIB_TYPE_VOLTAGE) ? s_adc : (int)(s_pmbus_i * 1000),
             (s_type == CALIB_TYPE_VOLTAGE) ? 0.0f : s_pmbus_i);

    char buf[256];
    snprintf(buf, sizeof(buf),
        "{\"cal_step\":%d,\"cal_target\":%.2f,\"cal_pwm\":%d,\"cal_raw\":%d}",
        s_step + 1,
        (s_type == CALIB_TYPE_VOLTAGE) ? s_data[s_step].target_v : s_data[s_step].target_i,
        s_pwm,
        (s_type == CALIB_TYPE_VOLTAGE) ? s_adc : (int)(s_pmbus_i * 1000));
    send_response(buf);

    s_step++;
    if (s_step >= CALIB_POINTS) { _on_complete(); return; }

    /* 进入下一步：动态计算 PWM 初始值 */
    float target = (s_type == CALIB_TYPE_VOLTAGE)
                   ? g_calib_v_targets[s_step]
                   : g_calib_i_targets[s_step];
    s_pwm = _calc_init_pwm(target, s_type == CALIB_TYPE_VOLTAGE);
    _apply_pwm(s_pwm);
    ESP_LOGI(TAG, "Step %d/%d: target=%.2f, PWM=%d",
             s_step+1, CALIB_POINTS, target, s_pwm);
}

static void _on_complete(void)
{
    ESP_LOGI(TAG, "===== Calibration COMPLETE =====");
    s_state = CALIB_STATE_COMPLETE;
    _save_to_nvs();

    /* 打印6点校准表 */
    for (int i = 0; i < CALIB_POINTS; i++) {
        ESP_LOGI(TAG, "  [%d] V=%.2f I=%.2f ADC=%d PMBus=%.4f PWM=%d",
                 i, s_data[i].target_v, s_data[i].target_i,
                 s_data[i].adc_raw, s_data[i].pmbus_i_raw, s_data[i].pwm_val);
    }
    calibration_stop();
}

static void _save_to_nvs(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_CALIB, NVS_READWRITE, &handle) != ESP_OK) return;
    nvs_set_blob(handle, "points", s_data, sizeof(s_data));
    nvs_commit(handle);
    nvs_close(handle);
}

static bool _load_from_nvs(void)
{
    nvs_handle_t handle;
    if (nvs_open(NVS_NS_CALIB, NVS_READONLY, &handle) != ESP_OK) return false;
    size_t sz = sizeof(s_data);
    esp_err_t ret = nvs_get_blob(handle, "points", s_data, &sz);
    nvs_close(handle);
    return (ret == ESP_OK);
}

/* ========== 缓存校准表（提高定时器内调用性能） ========== */
static struct {
    float raw[6];       /**< 电压用adc_raw / 电流用pmbus_i_raw */
    float tgt[6];       /**< 电压用target_v / 电流用target_i */
    int   count;
    bool  valid;        /**< 缓存是否就绪 */
} s_calib_cache = { .count = 0, .valid = false };

/**
 * @brief 刷新缓存校准表（在 calibration_start/on_complete/load 后调用）
 */
static void _rebuild_cache(void)
{
    s_calib_cache.count = 0;
    s_calib_cache.valid = false;

    for (int i = 0; i < CALIB_POINTS; i++) {
        /* 使用合并统一结构: 两种模式共用校准表，缓存由调用方通过 is_voltage 区分 */
        if (s_data[i].adc_raw > 0 || s_data[i].pmbus_i_raw > 0.001f) {
            s_calib_cache.raw[s_calib_cache.count] = (float)s_data[i].adc_raw;
            s_calib_cache.tgt[s_calib_cache.count] = s_data[i].target_v;
            s_calib_cache.count++;
            /* 注意: 目前缓存以电压模式为准，电流模式在校准时重建 */
        }
    }
    /* 也存一份电流的 */
    s_calib_cache.count = 0;
    for (int i = 0; i < CALIB_POINTS; i++) {
        if (s_data[i].pmbus_i_raw > 0.001f) {
            s_calib_cache.raw[s_calib_cache.count] = s_data[i].pmbus_i_raw;
            s_calib_cache.tgt[s_calib_cache.count] = s_data[i].target_i;
            s_calib_cache.count++;
        }
    }
    s_calib_cache.valid = (s_calib_cache.count >= 2);
    ESP_LOGD(TAG, "Cache rebuilt: %d points", s_calib_cache.count);
}

/* ========== 6点分段线性插值（统一API，使用静态缓存） ========== */

/**
 * @brief 线性插值核心（工业级数值稳定性）
 */
static inline float _lerp_stable(float x, float x0, float y0, float x1, float y1)
{
    float dx = x1 - x0;
    if (fabsf(dx) < 1e-6f) return y0;  /* 除以0保护 + 浮点容差 */
    return y0 + (x - x0) * (y1 - y0) / dx;
}

/**
 * @brief 执行分段线性插值计算
 * @param input         当前的原始输入值 (ADC raw 或 PMBus raw)
 * @param table         校准点数组 (需要按 target_v/i 从小到大排序)
 * @param num_points    校准点数量 (此处固定为 6)
 * @param is_voltage    标识是否为电压 (电压用 adc_raw，电流用 pmbus_i_raw)
 * @return float        计算出的标定值
 */
float calculate_calibrated_value(float input, const calib_point_t* table, int num_points, bool is_voltage)
{
    if (!table || num_points < 2) return 0.0f;

    /* 1. 提取 Step 0 的原始值做零点钳位 */
    float lower_raw = is_voltage ? (float)table[0].adc_raw : table[0].pmbus_i_raw;

    /* 2. 零点硬钳位：输入值 ≤ Step 0 读数 → 直接归零，彻底消除底噪 */
    if (input <= lower_raw) return 0.0f;

    /* 3. 查找输入值落在哪个区间 [i, i+1] */
    for (int i = 0; i < num_points - 1; i++) {
        float raw_i    = is_voltage ? (float)table[i].adc_raw   : table[i].pmbus_i_raw;
        float raw_next = is_voltage ? (float)table[i+1].adc_raw : table[i+1].pmbus_i_raw;
        if (input < raw_i) return 0.0f;             /* 非单调→安全归零 */
        if (input >= raw_i && input <= raw_next) {
            float tgt_i    = is_voltage ? table[i].target_v         : table[i].target_i;
            float tgt_next = is_voltage ? table[i+1].target_v       : table[i+1].target_i;
            return _lerp_stable(input, raw_i, tgt_i, raw_next, tgt_next);
        }
    }

    /* 4. 外推：输入值超过最大校准点 → 用最后两点斜率外推 */
    int last = num_points - 1;
    int prev = num_points - 2;
    float raw_last  = is_voltage ? (float)table[last].adc_raw   : table[last].pmbus_i_raw;
    float raw_prev  = is_voltage ? (float)table[prev].adc_raw   : table[prev].pmbus_i_raw;
    float tgt_last  = is_voltage ? table[last].target_v         : table[last].target_i;
    float tgt_prev  = is_voltage ? table[prev].target_v         : table[prev].target_i;

    return _lerp_stable(input, raw_last, tgt_last, raw_last + (raw_last - raw_prev), tgt_last + (tgt_last - tgt_prev));
}

/* 保留旧函数名作为兼容桥接 */
float calculate_calibrated_v(int adc_raw, const calib_point_t* points)
{
    return calculate_calibrated_value((float)adc_raw, points, CALIB_POINTS, true);
}

float calculate_calibrated_i(float pmbus_raw, const calib_point_t* points)
{
    return calculate_calibrated_value(pmbus_raw, points, CALIB_POINTS, false);
}
