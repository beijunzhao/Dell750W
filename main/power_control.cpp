/**
 * power_control.cpp - PowerControl 类实现
 *
 * V_set / I_set 保存到 NVS 自动恢复
 */
#include "power_control.h"
#include "pmbus.h"
#include <cmath>
#include "esp_log.h"

static const char* TAG = "PowerCtrl";
#define NVS_NS_POWER "power_ctrl"

// 静态成员初始化
float PowerControl::_voltageSet = 0.0f;
float PowerControl::_currentSet = 0.0f;
bool  PowerControl::_powerState = false;
float PowerControl::_vMax = PSU_VOLTAGE_MAX_DEF;
float PowerControl::_iMax = PSU_CURRENT_MAX_DEF;

/** 保存全部参数到 NVS */
static void _save_all_nvs(float v, float i, float vmax, float imax)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS_POWER, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, "v", &v, sizeof(v));
    nvs_set_blob(h, "i", &i, sizeof(i));
    nvs_set_blob(h, "vmax", &vmax, sizeof(vmax));
    nvs_set_blob(h, "imax", &imax, sizeof(imax));
    nvs_commit(h);
    nvs_close(h);
    ESP_LOGD(TAG, "NVS saved: V=%.3f I=%.3f Vmax=%.1f Imax=%.1f", v, i, vmax, imax);
}

/** 从 NVS 读取全部参数 */
static void _load_all_nvs(float* v, float* i, float* vmax, float* imax)
{
    *v = 0; *i = 0; *vmax = PSU_VOLTAGE_MAX_DEF; *imax = PSU_CURRENT_MAX_DEF;
    nvs_handle_t h;
    if (nvs_open(NVS_NS_POWER, NVS_READONLY, &h) != ESP_OK) return;
    size_t sz = sizeof(float);
    nvs_get_blob(h, "v", v, &sz);
    sz = sizeof(float);
    nvs_get_blob(h, "i", i, &sz);
    sz = sizeof(float);
    nvs_get_blob(h, "vmax", vmax, &sz);
    sz = sizeof(float);
    nvs_get_blob(h, "imax", imax, &sz);
    nvs_close(h);
}

esp_err_t PowerControl::init()
{
    esp_err_t ret;

    ret = _gpioInitLow(SW_CTRL);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Failed to init SW_CTRL"); return ret; }
    ESP_LOGI(TAG, "SW_CTRL initialized LOW (safe)");

    ret = _pwmInit(V_PWM, LEDC_CHANNEL_0, 0);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Failed to init V_PWM"); return ret; }

    ret = _pwmInit(I_PWM, LEDC_CHANNEL_1, 0);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Failed to init I_PWM"); return ret; }

    _powerState = false;

    /* 从 NVS 读取上次保存的全部参数 */
    _load_all_nvs(&_voltageSet, &_currentSet, &_vMax, &_iMax);
    if (_voltageSet > 0.001f || _currentSet > 0.001f) {
        ESP_LOGI(TAG, "Restored: V_set=%.3f V, I_set=%.3f A", _voltageSet, _currentSet);
    }

    /* 恢复 PWM 输出（不开机，只恢复 DAC 偏置） */
    if (_voltageSet > 0.001f) {
        float vDac = (_voltageSet / _vMax) * V_DAC_MAX;
        uint32_t duty = (uint32_t)((vDac / MCU_VDD) * PWM_MAX_DUTY);
        if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
        _pwmSetDuty(LEDC_CHANNEL_0, duty);
    }
    if (_currentSet > 0.001f) {
        float iDac = (_currentSet / _iMax) * V_DAC_MAX;
        uint32_t duty = (uint32_t)((iDac / MCU_VDD) * PWM_MAX_DUTY);
        if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
        _pwmSetDuty(LEDC_CHANNEL_1, duty);
    }

    return ESP_OK;
}

void PowerControl::powerOn()
{
    gpio_set_level(SW_CTRL, 1);
    _powerState = true;
    ESP_LOGI(TAG, "Power ON");
}

void PowerControl::powerOff()
{
    gpio_set_level(SW_CTRL, 0);
    _powerState = false;
    ESP_LOGI(TAG, "Power OFF");
}

bool PowerControl::isPoweredOn()
{
    return _powerState;
}

#define V_DEAD_ZONE_LOW  0.1f   /* 死区下限 */
#define V_DEAD_ZONE_HIGH 2.5f   /* 死区上限 */

esp_err_t PowerControl::setVoltage(float voltage)
{
    /* 死区跳变：0.1V~2.5V 区间存在物理震荡，强制跳过 */
    if (voltage > V_DEAD_ZONE_LOW && voltage < V_DEAD_ZONE_HIGH) {
        voltage = V_DEAD_ZONE_HIGH;  /* 拉高到2.5V跳过死区 */
        ESP_LOGI(TAG, "Voltage %.3f in dead zone, forced to %.1fV", _voltageSet, V_DEAD_ZONE_HIGH);
    }
    if (voltage < 0.0f || voltage > _vMax) {
        ESP_LOGW(TAG, "Voltage out of range: %.3f (valid: 0~%.1f)", voltage, _vMax);
        return ESP_ERR_INVALID_ARG;
    }
    _voltageSet = voltage;
    float vDac = (voltage / _vMax) * V_DAC_MAX;
    uint32_t duty = (uint32_t)((vDac / MCU_VDD) * PWM_MAX_DUTY);
    if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
    _pwmSetDuty(LEDC_CHANNEL_0, duty);
    _save_all_nvs(_voltageSet, _currentSet, _vMax, _iMax);
    return ESP_OK;
}

esp_err_t PowerControl::setCurrent(float current)
{
    if (current < 0.0f || current > _iMax) {
        ESP_LOGW(TAG, "Current out of range: %.3f (valid: 0~%.1f)", current, _iMax);
        return ESP_ERR_INVALID_ARG;
    }
    _currentSet = current;
    float iDac = (current / _iMax) * V_DAC_MAX;
    uint32_t duty = (uint32_t)((iDac / MCU_VDD) * PWM_MAX_DUTY);
    if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
    _pwmSetDuty(LEDC_CHANNEL_1, duty);
    _save_all_nvs(_voltageSet, _currentSet, _vMax, _iMax);
    return ESP_OK;
}

void PowerControl::setVMax(float vMax)
{
    if (vMax < 0.0f) vMax = 0.0f;
    if (vMax > PSU_VOLTAGE_MAX) vMax = PSU_VOLTAGE_MAX;
    _vMax = vMax;
    _save_all_nvs(_voltageSet, _currentSet, _vMax, _iMax);
    ESP_LOGI(TAG, "V_MAX set to %.1fV", _vMax);
}

void PowerControl::setIMax(float iMax)
{
    if (iMax < 0.0f) iMax = 0.0f;
    if (iMax > PSU_CURRENT_MAX) iMax = PSU_CURRENT_MAX;
    _iMax = iMax;
    _save_all_nvs(_voltageSet, _currentSet, _vMax, _iMax);
    ESP_LOGI(TAG, "I_MAX set to %.1fA", _iMax);
}

void PowerControl::setPwmDuty(ledc_channel_t channel, uint32_t duty)
{
    if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
    _pwmSetDuty(channel, duty);
}

// ---------- 私有方法 ----------

esp_err_t PowerControl::_gpioInitLow(gpio_num_t gpio)
{
    gpio_config_t cfg = {};
    cfg.pin_bit_mask = (1ULL << gpio);
    cfg.mode         = GPIO_MODE_OUTPUT;
    cfg.pull_up_en   = GPIO_PULLUP_DISABLE;
    cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    cfg.intr_type    = GPIO_INTR_DISABLE;
    esp_err_t ret = gpio_config(&cfg);
    if (ret != ESP_OK) return ret;
    gpio_set_level(gpio, 0);
    return ESP_OK;
}

esp_err_t PowerControl::_pwmInit(gpio_num_t gpio, ledc_channel_t channel, uint32_t duty)
{
    static bool timerInited = false;
    if (!timerInited) {
        ledc_timer_config_t timerCfg = {};
        timerCfg.speed_mode      = LEDC_LOW_SPEED_MODE;
        timerCfg.timer_num       = LEDC_TIMER_0;
        timerCfg.duty_resolution = (ledc_timer_bit_t)PWM_RES_BITS;
        timerCfg.freq_hz         = PWM_FREQ_HZ;
        timerCfg.clk_cfg         = LEDC_USE_APB_CLK;
        esp_err_t ret = ledc_timer_config(&timerCfg);
        if (ret != ESP_OK) { ESP_LOGE(TAG, "LEDC timer config failed"); return ret; }
        timerInited = true;
    }
    ledc_channel_config_t chanCfg = {};
    chanCfg.speed_mode = LEDC_LOW_SPEED_MODE;
    chanCfg.channel    = channel;
    chanCfg.timer_sel  = LEDC_TIMER_0;
    chanCfg.intr_type  = LEDC_INTR_DISABLE;
    chanCfg.gpio_num   = (int)gpio;
    chanCfg.duty       = duty;
    chanCfg.hpoint     = 0;
    return ledc_channel_config(&chanCfg);
}

void PowerControl::_pwmSetDuty(ledc_channel_t channel, uint32_t duty)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}