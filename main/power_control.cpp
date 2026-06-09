/**
 * power_control.cpp - PowerControl 类实现
 *
 * 所有静态变量受互斥锁保护。
 * setVoltage/setCurrent 不再立即写入 NVS，
 * 由应用层通过 saveToNVS() 按需保存。
 */
#include "power_control.h"
#include "pmbus.h"
#include "calibration.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <cmath>

static const char* TAG = "PowerCtrl";
#define NVS_NS_POWER "power_ctrl"

/* 死区阈值（硬件特性：0.1V~2.5V 存在物理震荡） */
#define V_DEAD_ZONE_LOW  0.1f
#define V_DEAD_ZONE_HIGH 2.5f

/* 静态成员初始化 */
float PowerControl::_voltageSet = 0.0f;
float PowerControl::_currentSet = 0.0f;
bool  PowerControl::_powerState = false;
float PowerControl::_vMax = PSU_VOLTAGE_MAX_DEF;
float PowerControl::_iMax = PSU_CURRENT_MAX_DEF;
static bool s_initialized = false;
static SemaphoreHandle_t s_mutex = nullptr;

/* ========== 内部辅助：NVS 读写 ========== */

static void _load_all_nvs(float* v, float* i, float* vmax, float* imax)
{
    *v = 0; *i = 0; *vmax = PSU_VOLTAGE_MAX_DEF; *imax = PSU_CURRENT_MAX_DEF;
    nvs_handle_t h;
    if (nvs_open(NVS_NS_POWER, NVS_READONLY, &h) != ESP_OK) return;
    size_t sz = sizeof(float);
    nvs_get_blob(h, "v", v, &sz);
    sz = sizeof(float); nvs_get_blob(h, "i", i, &sz);
    sz = sizeof(float); nvs_get_blob(h, "vmax", vmax, &sz);
    sz = sizeof(float); nvs_get_blob(h, "imax", imax, &sz);
    nvs_close(h);
}

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
    ESP_LOGI(TAG, "NVS saved: V=%.3f I=%.3f Vmax=%.1f Imax=%.1f", v, i, vmax, imax);
}

/* ========== 初始化 ========== */

esp_err_t PowerControl::init()
{
    if (s_initialized) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }

    s_mutex = xSemaphoreCreateMutex();
    if (!s_mutex) { ESP_LOGE(TAG, "Mutex create failed"); return ESP_ERR_NO_MEM; }

    esp_err_t ret = _gpioInitLow(SW_CTRL);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "SW_CTRL init failed"); return ret; }

    ret = _pwmInit(V_PWM, LEDC_CHANNEL_0, 0);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "V_PWM init failed"); return ret; }

    ret = _pwmInit(I_PWM, LEDC_CHANNEL_1, 0);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "I_PWM init failed"); return ret; }

    _powerState = false;

    /* 从 NVS 加载参数（初始化阶段单线程，加锁保持一致性） */
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        _load_all_nvs(&_voltageSet, &_currentSet, &_vMax, &_iMax);
        xSemaphoreGive(s_mutex);
    }
    ESP_LOGI(TAG, "Loaded: V_set=%.3f I_set=%.3f Vmax=%.1f Imax=%.1f",
             _voltageSet, _currentSet, _vMax, _iMax);

    /* 恢复 PWM 偏置：使用校准表或公式回退 */
    if (_voltageSet > 0.001f) {
        int duty = calibration_v_to_pwm(_voltageSet);
        if (duty >= 0) _pwmSetDuty(LEDC_CHANNEL_0, (uint32_t)duty);
    }
    if (_currentSet > 0.001f) {
        int duty = calibration_i_to_pwm(_currentSet);
        if (duty >= 0) _pwmSetDuty(LEDC_CHANNEL_1, (uint32_t)duty);
    }

    s_initialized = true;
    ESP_LOGI(TAG, "PowerControl initialized (mutex protected)");
    return ESP_OK;
}

void PowerControl::saveToNVS()
{
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        _save_all_nvs(_voltageSet, _currentSet, _vMax, _iMax);
        xSemaphoreGive(s_mutex);
    } else {
        ESP_LOGE(TAG, "saveToNVS: failed to acquire mutex");
    }
}

/* ========== 开关机 ========== */

void PowerControl::powerOn()
{
    gpio_set_level(SW_CTRL, 1);
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        _powerState = true;
        xSemaphoreGive(s_mutex);
    }
    ESP_LOGI(TAG, "Power ON");
}

void PowerControl::powerOff()
{
    gpio_set_level(SW_CTRL, 0);
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        _powerState = false;
        xSemaphoreGive(s_mutex);
    }
    ESP_LOGI(TAG, "Power OFF");
}

bool PowerControl::isPoweredOn()
{
    bool state = false;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        state = _powerState;
        xSemaphoreGive(s_mutex);
    }
    return state;
}

/* ========== 设定电压（原子性：变量更新+PWM写入均在锁内） ========== */

esp_err_t PowerControl::setVoltage(float voltage)
{
    /* 死区跳变 */
    if (voltage > V_DEAD_ZONE_LOW && voltage < V_DEAD_ZONE_HIGH) {
        ESP_LOGI(TAG, "Voltage %.3f in dead zone, forced to %.1fV", voltage, V_DEAD_ZONE_HIGH);
        voltage = V_DEAD_ZONE_HIGH;
    }

    int duty = calibration_v_to_pwm(voltage);
    if (duty < 0) {
        ESP_LOGE(TAG, "calibration_v_to_pwm returned %d, aborting", duty);
        return ESP_FAIL;
    }

    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdTRUE) {
        return ESP_FAIL;
    }

    /* 再次检查范围（因为 _vMax 可能在锁外被修改，但概率极低；安全起见在锁内复核） */
    if (voltage < 0.0f || voltage > _vMax) {
        xSemaphoreGive(s_mutex);
        ESP_LOGW(TAG, "Voltage %.3f out of range (0~%.1f)", voltage, _vMax);
        return ESP_ERR_INVALID_ARG;
    }

    _voltageSet = voltage;
    _pwmSetDuty(LEDC_CHANNEL_0, (uint32_t)duty);
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

esp_err_t PowerControl::setCurrent(float current)
{
    if (current < 0.0f || current > _iMax) {
        ESP_LOGW(TAG, "Current %.3f out of range (0~%.1f)", current, _iMax);
        return ESP_ERR_INVALID_ARG;
    }

    int duty = calibration_i_to_pwm(current);
    if (duty < 0) {
        ESP_LOGE(TAG, "calibration_i_to_pwm returned %d, aborting", duty);
        return ESP_FAIL;
    }

    if (xSemaphoreTake(s_mutex, portMAX_DELAY) != pdTRUE) {
        return ESP_FAIL;
    }

    if (current > _iMax) {
        xSemaphoreGive(s_mutex);
        return ESP_ERR_INVALID_ARG;
    }

    _currentSet = current;
    _pwmSetDuty(LEDC_CHANNEL_1, (uint32_t)duty);
    xSemaphoreGive(s_mutex);
    return ESP_OK;
}

/* ========== 量程设置（避免递归锁） ========== */

void PowerControl::setVMax(float vMax)
{
    if (vMax < 0.0f) vMax = 0.0f;
    if (vMax > PSU_VOLTAGE_MAX) vMax = PSU_VOLTAGE_MAX;

    bool needUpdate = false;
    float newVoltage = 0.0f;

    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        float oldV = _voltageSet;
        _vMax = vMax;
        if (_voltageSet > vMax) {
            _voltageSet = vMax;
            newVoltage = vMax;
            needUpdate = true;
            ESP_LOGI(TAG, "V_set trimmed from %.3f to %.1f", oldV, vMax);
        }
        xSemaphoreGive(s_mutex);
    }

    if (needUpdate) {
        /* 锁外更新 PWM，避免嵌套加锁 */
        int duty = calibration_v_to_pwm(newVoltage);
        if (duty >= 0) {
            _pwmSetDuty(LEDC_CHANNEL_0, (uint32_t)duty);
        }
    }
    saveToNVS();
}

void PowerControl::setIMax(float iMax)
{
    if (iMax < 0.0f) iMax = 0.0f;
    if (iMax > PSU_CURRENT_MAX) iMax = PSU_CURRENT_MAX;

    bool needUpdate = false;
    float newCurrent = 0.0f;

    if (xSemaphoreTake(s_mutex, portMAX_DELAY) == pdTRUE) {
        float oldI = _currentSet;
        _iMax = iMax;
        if (_currentSet > iMax) {
            _currentSet = iMax;
            newCurrent = iMax;
            needUpdate = true;
            ESP_LOGI(TAG, "I_set trimmed from %.3f to %.1f", oldI, iMax);
        }
        xSemaphoreGive(s_mutex);
    }

    if (needUpdate) {
        int duty = calibration_i_to_pwm(newCurrent);
        if (duty >= 0) {
            _pwmSetDuty(LEDC_CHANNEL_1, (uint32_t)duty);
        }
    }
    saveToNVS();
}

void PowerControl::setPwmDuty(ledc_channel_t channel, uint32_t duty)
{
    if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
    // 直接设置硬件，无共享变量影响，无需锁
    _pwmSetDuty(channel, duty);
}

/* ========== 读取函数（加锁） ========== */

float PowerControl::getSetVoltage()
{
    float v = 0;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100)) == pdTRUE) { v = _voltageSet; xSemaphoreGive(s_mutex); }
    return v;
}

float PowerControl::getSetCurrent()
{
    float v = 0;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100)) == pdTRUE) { v = _currentSet; xSemaphoreGive(s_mutex); }
    return v;
}

float PowerControl::getVMax()
{
    float v = PSU_VOLTAGE_MAX_DEF;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100)) == pdTRUE) { v = _vMax; xSemaphoreGive(s_mutex); }
    return v;
}

float PowerControl::getIMax()
{
    float v = PSU_CURRENT_MAX_DEF;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(100)) == pdTRUE) { v = _iMax; xSemaphoreGive(s_mutex); }
    return v;
}

/* ========== 私有方法 ========== */

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
        if (ret != ESP_OK) return ret;
        timerInited = true;
    }
    ledc_channel_config_t chanCfg = {};
    chanCfg.speed_mode = LEDC_LOW_SPEED_MODE;
    chanCfg.channel    = channel;
    chanCfg.timer_sel  = LEDC_TIMER_0;
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