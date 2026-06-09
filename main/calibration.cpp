/**
 * calibration.cpp - 6点校准模块
 *
 * ===== 核心思路 =====
 * 校准就是记录 (目标值, 对应的PWM) 6组数据。
 * 设置电压/电流时，查校准表插值得到PWM后输出。
 * ADC只用来显示，不参与校准计算。
 *
 * ===== 数据流 =====
 * 校准: 用户调PWM使万用表=3V → 记下 (target=3V, pwm=XXX)
 * 设值: 用户设5V → 查表插值 → 输出PWM → 万用表显示5V
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "Calib";
#define NVS_NS "calib_data"
#define PWM_STEP  5

/* 默认校准目标值 */
const float g_calib_v_targets[CALIB_POINTS] = {0.0f, 3.0f, 5.0f, 8.0f, 10.0f, 12.0f};
const float g_calib_i_targets[CALIB_POINTS] = {0.0f, 10.0f, 20.0f, 30.0f, 50.0f, 62.5f};

/* ---- 状态 ---- */
static calib_state_t  s_state = CALIB_STATE_IDLE;
static calib_type_t   s_type  = CALIB_TYPE_VOLTAGE;
static int            s_step  = 0;
static int            s_pwm   = 0;
static int            s_adc   = 0;
static float          s_pmbus_i = 0.0f;
static int            s_adc_log[CALIB_POINTS]; /* 电压校准时记录每步ADC */

/* 电压/电流独立校准表 */
static calib_point_t  s_tbl_v[CALIB_POINTS];  /* 电压校准表 */
static calib_point_t  s_tbl_i[CALIB_POINTS];  /* 电流校准表 */
static int            s_adc_tbl[CALIB_POINTS]; /* ADC校准表: target_v对应的raw_adc */

static void _apply_pwm(int pwm);
static void _on_step_confirm(void);
static void _on_complete(void);
static void _save_nvs(void);
static void _load_nvs(void);

/* ========== PWM 初始值计算（前向声明，用于校准第一步） ========== */
static int _calc_init_pwm(float target, bool is_v);

/* ========== 公开 API ========== */

void calibration_init(void)
{
    _load_nvs();
    ESP_LOGI(TAG, "Calibration module initialized");
}

void calibration_start_v(void)
{
    if (s_state != CALIB_STATE_IDLE) return;
    if (!PowerControl::isPoweredOn()) {
        PowerControl::powerOn();
        vTaskDelay(pdMS_TO_TICKS(200));
        ESP_LOGI(TAG, "Auto power-on for voltage calibration");
    }

    ESP_LOGI(TAG, "===== Voltage Calibration START =====");
    s_type = CALIB_TYPE_VOLTAGE;
    s_step = 0;
    s_state = CALIB_STATE_ADJUSTING;
    lvgl_show_calibration();

    s_pwm = _calc_init_pwm(g_calib_v_targets[0], true);
    s_adc = ADCSampler::getRawAdc();

    /* 初始化电压校准表，不清除电流表 */
    for (int i = 0; i < CALIB_POINTS; i++) {
        s_tbl_v[i].target_v = g_calib_v_targets[i];
        s_tbl_v[i].target_i = 0;
        s_tbl_v[i].pwm_val  = 0;
    }
    _apply_pwm(s_pwm);
}

void calibration_start_i(void)
{
    if (s_state != CALIB_STATE_IDLE) return;
    if (!PowerControl::isPoweredOn()) {
        PowerControl::powerOn();
        vTaskDelay(pdMS_TO_TICKS(200));
        ESP_LOGI(TAG, "Auto power-on for current calibration");
    }

    /* 电流校准时将电压设为 2.7V，降低功率避免过热 */
    PowerControl::setVoltage(2.7f);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "===== Current Calibration START =====");
    s_type = CALIB_TYPE_CURRENT;
    s_step = 0;
    s_state = CALIB_STATE_ADJUSTING;
    lvgl_show_current_calibration();

    s_pwm = _calc_init_pwm(g_calib_i_targets[0], false);
    s_pmbus_i = PMBus::I_out;

    for (int i = 0; i < CALIB_POINTS; i++) {
        s_tbl_i[i].target_v = 0;
        s_tbl_i[i].target_i = g_calib_i_targets[i];
        s_tbl_i[i].pwm_val  = 0;
    }
    _apply_pwm(s_pwm);
}

void calibration_stop(void)
{
    if (s_state == CALIB_STATE_IDLE) return;
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

    if (btn_up) { s_pwm += PWM_STEP; if (s_pwm > PWM_MAX_DUTY) s_pwm = PWM_MAX_DUTY; _apply_pwm(s_pwm); }
    else if (btn_down) { s_pwm -= PWM_STEP; if (s_pwm < 0) s_pwm = 0; _apply_pwm(s_pwm); }
    else if (ok_rising) { _on_step_confirm(); }
}

void calibration_update_adc(int raw_adc) { if (s_state == CALIB_STATE_ADJUSTING && s_type == CALIB_TYPE_VOLTAGE) s_adc = raw_adc; }
void calibration_update_pmbus(float current) { if (s_state == CALIB_STATE_ADJUSTING && s_type == CALIB_TYPE_CURRENT) s_pmbus_i = current; }

const calib_point_t* calibration_get_data(void) { return (s_type == CALIB_TYPE_VOLTAGE) ? s_tbl_v : s_tbl_i; }
int calibration_get_current_step(void) { return (s_state == CALIB_STATE_IDLE) ? -1 : s_step; }
int calibration_get_pwm(void) { return s_pwm; }
float calibration_get_pmbus_value(void) { return s_pmbus_i; }

/* ========== PWM 初始值计算（临时，仅校准第一步用） ========== */

static int _calc_init_pwm(float target, bool is_v)
{
    float rmax = is_v ? PowerControl::getVMax() : PowerControl::getIMax();
    if (rmax <= 0.1f) rmax = is_v ? PSU_VOLTAGE_MAX : PSU_CURRENT_MAX;
    float vdac = (target / rmax) * V_DAC_MAX;
    int pwm = (int)((vdac / MCU_VDD) * PWM_MAX_DUTY);
    if (pwm < 0) pwm = 0;
    if (pwm > PWM_MAX_DUTY) pwm = PWM_MAX_DUTY;
    return pwm;
}

/* ========== 私有函数 ========== */

static void _apply_pwm(int pwm)
{
    ledc_channel_t ch = (s_type == CALIB_TYPE_VOLTAGE) ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1;
    PowerControl::setPwmDuty(ch, (uint32_t)pwm);
}

extern void send_response(const char* json);

static void _on_step_confirm(void)
{
    calib_point_t* tbl = (s_type == CALIB_TYPE_VOLTAGE) ? s_tbl_v : s_tbl_i;

    tbl[s_step].pwm_val = s_pwm;  /* ★ 核心：保存当前PWM值 */
    if (s_type == CALIB_TYPE_VOLTAGE) s_adc_log[s_step] = s_adc;  /* 记录ADC用于校准 */

    float tgt = (s_type == CALIB_TYPE_VOLTAGE) ? tbl[s_step].target_v : tbl[s_step].target_i;
    ESP_LOGI(TAG, "Step %d done: target=%.2f PWM=%d", s_step+1, tgt, s_pwm);

    char resp_buf[256];
    if (s_type == CALIB_TYPE_VOLTAGE) {
        snprintf(resp_buf, sizeof(resp_buf),
            "{\"cal_result_target\":%.2f,\"cal_result_adc\":%d,\"cal_result_pwm\":%d}", tgt, s_adc, s_pwm);
    } else {
        snprintf(resp_buf, sizeof(resp_buf),
            "{\"cal_result_target\":%.2f,\"cal_result_adc\":%.3f,\"cal_result_pwm\":%d}", tgt, s_pmbus_i, s_pwm);
    }
    send_response(resp_buf);

    s_step++;
    if (s_step >= CALIB_POINTS) { _on_complete(); return; }

    /* 进入下一步：用目标值对应的校准PWM（如果没有则用公式倒算） */
    float target = (s_type == CALIB_TYPE_VOLTAGE) ? g_calib_v_targets[s_step] : g_calib_i_targets[s_step];

    /* 优先用已有校准数据，没有才公式倒算 */
    if (s_type == CALIB_TYPE_VOLTAGE) {
        s_pwm = calibration_v_to_pwm(target);
    } else {
        s_pwm = calibration_i_to_pwm(target);
    }
    _apply_pwm(s_pwm);
}

static void _on_complete(void)
{
    ESP_LOGI(TAG, "===== Calibration COMPLETE =====");
    s_state = CALIB_STATE_COMPLETE;

    /* 1. 先打印当前步骤收集到的结果 */
    calib_point_t* tbl = (s_type == CALIB_TYPE_VOLTAGE) ? s_tbl_v : s_tbl_i;
    for (int i = 0; i < CALIB_POINTS; i++) {
        float tgt = (s_type == CALIB_TYPE_VOLTAGE) ? tbl[i].target_v : tbl[i].target_i;
        ESP_LOGI(TAG, "  [%d] target=%.2f PWM=%d ADC=%d", i, tgt, tbl[i].pwm_val, s_adc_log[i]);
    }

    /* 2. 核心修复：必须先更新内存里的 ADC 校准表！ */
    if (s_type == CALIB_TYPE_VOLTAGE) {
        for (int i = 0; i < CALIB_POINTS; i++) {
            s_adc_tbl[i] = s_adc_log[i];
        }
        ESP_LOGI(TAG, "ADC cal table RAM updated: [%d %d %d %d %d %d]",
                 s_adc_tbl[0], s_adc_tbl[1], s_adc_tbl[2],
                 s_adc_tbl[3], s_adc_tbl[4], s_adc_tbl[5]);
    }

    /* 3. 最后再执行保存，将包含最新数据的数组写入 NVS */
    _save_nvs();

    calibration_stop();
}

/* ========== NVS 持久化 ========== */

static void _save_nvs(void)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, "tbl_v", s_tbl_v, sizeof(s_tbl_v));
    nvs_set_blob(h, "tbl_i", s_tbl_i, sizeof(s_tbl_i));
    nvs_set_blob(h, "adc_tbl", s_adc_tbl, sizeof(s_adc_tbl));
    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "Calibration tables saved to NVS (PWM + ADC)");
}

static void _load_nvs(void)
{
    memset(s_tbl_v, 0, sizeof(s_tbl_v));
    memset(s_tbl_i, 0, sizeof(s_tbl_i));
    memset(s_adc_tbl, 0, sizeof(s_adc_tbl));

    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) == ESP_OK) {
        size_t sz = sizeof(s_tbl_v);
        nvs_get_blob(h, "tbl_v", s_tbl_v, &sz);
        sz = sizeof(s_tbl_i);
        nvs_get_blob(h, "tbl_i", s_tbl_i, &sz);
        sz = sizeof(s_adc_tbl);
        nvs_get_blob(h, "adc_tbl", s_adc_tbl, &sz);
        nvs_close(h);
        ESP_LOGI(TAG, "Calibration tables loaded from NVS");
    }
}

/* ========== ★ 核心：校准查表 → PWM ========== */

/**
 * 查表插值：输入目标值，输出对应 PWM
 * 规则：
 *   target <= table[0].target → PWM = table[0].pwm_val（关断态）
 *   区间内线性插值
 *   超出范围 → 最后两点外推
 *   无校准数据（所有pwm=0）→ 返回 -1 表示需用公式
 */
static int _table_to_pwm(float target, const calib_point_t* tbl)
{
    /* 构建有效点数组 */
    float tgt[6];
    int   pwm[6];
    int cnt = 0;

    for (int i = 0; i < CALIB_POINTS; i++) {
        if (tbl[i].pwm_val > 0 || i == 0) {  /* 第0点允许pwm=0 */
            tgt[cnt] = (tbl[i].target_v > 0.01f) ? tbl[i].target_v : tbl[i].target_i;
            pwm[cnt] = tbl[i].pwm_val;
            cnt++;
        }
    }

    /* 无有效校准数据 → 返回 -1 让调用方用公式 */
    if (cnt < 2) return -1;

    /* 小于等于第0点 → 返回第0点的PWM */
    if (target <= tgt[0]) return pwm[0];

    /* 区间查找 */
    for (int i = 0; i < cnt - 1; i++) {
        if (target >= tgt[i] && target <= tgt[i+1]) {
            float dx = tgt[i+1] - tgt[i];
            if (fabsf(dx) < 1e-6f) return pwm[i];
            return pwm[i] + (int)((target - tgt[i]) * (pwm[i+1] - pwm[i]) / dx);
        }
    }

    /* 外推：最后两点 */
    int last = cnt - 1, prev = cnt - 2;
    float dx = tgt[last] - tgt[prev];
    if (fabsf(dx) < 1e-6f) return pwm[last];
    return pwm[last] + (int)((target - tgt[last]) * (pwm[last] - pwm[prev]) / dx);
}

int calibration_v_to_pwm(float target_v)
{
    int pwm = _table_to_pwm(target_v, s_tbl_v);
    if (pwm >= 0) return pwm;

    /* 无校准数据 → 用公式倒算 */
    float vmax = PowerControl::getVMax();
    if (vmax <= 0.1f) vmax = PSU_VOLTAGE_MAX;
    if (target_v <= 0.1f) return 0;
    float vdac = (target_v / vmax) * V_DAC_MAX;
    pwm = (int)((vdac / MCU_VDD) * PWM_MAX_DUTY);
    if (pwm > PWM_MAX_DUTY) pwm = PWM_MAX_DUTY;
    return pwm;
}

/* ========== ADC 校准: mV → 电压 (6点插值) ========== */

float calculate_calibrated_value(float input_mv, bool is_voltage)
{
    if (!is_voltage) return input_mv;

    /* 构建有效点: (mV, target_v) */
    float adc_pts[6], v_pts[6];
    int cnt = 0;
    for (int i = 0; i < CALIB_POINTS; i++) {
        if (s_adc_tbl[i] > 0 || i == 0) {
            adc_pts[cnt] = (float)s_adc_tbl[i];
            v_pts[cnt] = g_calib_v_targets[i];
            cnt++;
        }
    }
    /* 无校准数据 → 用公式（输入为原始 ADC raw） */
    if (cnt < 2) {
        float v = (input_mv / 4095.0f) * MCU_VDD * ADC_DIVIDER_RATIO;
        return v;
    }
    /* 低于第0点 */
    if (input_mv <= adc_pts[0]) return v_pts[0];
    /* 区间插值 */
    for (int i = 0; i < cnt - 1; i++) {
        if (input_mv >= adc_pts[i] && input_mv <= adc_pts[i+1]) {
            float dx = adc_pts[i+1] - adc_pts[i];
            if (fabsf(dx) < 1e-6f) return v_pts[i];
            return v_pts[i] + (input_mv - adc_pts[i]) * (v_pts[i+1] - v_pts[i]) / dx;
        }
    }
    /* 外推: 最后两点 */
    int last = cnt - 1, prev = cnt - 2;
    float dx = adc_pts[last] - adc_pts[prev];
    if (fabsf(dx) < 1e-6f) return v_pts[last];
    return v_pts[last] + (input_mv - adc_pts[last]) * (v_pts[last] - v_pts[prev]) / dx;
}

int calibration_i_to_pwm(float target_i)
{
    int pwm = _table_to_pwm(target_i, s_tbl_i);
    if (pwm >= 0) return pwm;

    float imax = PowerControl::getIMax();
    if (imax <= 0.1f) imax = PSU_CURRENT_MAX;
    if (target_i <= 0.01f) return 0;
    float idac = (target_i / imax) * V_DAC_MAX;
    pwm = (int)((idac / MCU_VDD) * PWM_MAX_DUTY);
    if (pwm > PWM_MAX_DUTY) pwm = PWM_MAX_DUTY;
    return pwm;
}