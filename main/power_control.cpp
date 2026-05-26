/**
 * power_control.cpp - PowerControl 类实现
 */
#include "power_control.h"
#include "pmbus.h"
#include <cmath>
#include "esp_log.h"

static const char* TAG = "PowerCtrl";

// 静态成员初始化
float PowerControl::_voltageSet = 0.0f;
float PowerControl::_currentSet = 0.0f;
bool  PowerControl::_powerState = false;

esp_err_t PowerControl::init()
{
    esp_err_t ret;

    // 第一步: 锁定命门 - SW_CTRL 输出 LOW (切断电源输出)
    ret = _gpioInitLow(SW_CTRL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init SW_CTRL");
        return ret;
    }
    ESP_LOGI(TAG, "SW_CTRL initialized LOW (safe)");

    // 第二步: 安全偏置
    // V_PWM 输出 0% 占空比 (0V DAC → 0V 物理输出)
    ret = _pwmInit(V_PWM, LEDC_CHANNEL_0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init V_PWM");
        return ret;
    }

    // I_PWM 输出 0% 占空比 (0A 限流)
    ret = _pwmInit(I_PWM, LEDC_CHANNEL_1, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init I_PWM");
        return ret;
    }

    ESP_LOGI(TAG, "PWM initialized: V_PWM=0%% (safe: ~0V output), I_PWM=0%%");

    _powerState = false;
    _voltageSet = 0.0f;
    _currentSet = 0.0f;

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
    // 监控模式: 电源一直由硬件控制 (如 PSON/PSKILL 硬线连接),
    // ESP32 只做监控不控制开关。
    //
    // 判断依据:
    //   1. 如果 PMBus 在线且 V_out > 1V, 认为电源已开启
    //   2. 如果 PMBus 在线但 V_out <= 1V, 检查 GPIO 状态
    //      (刚开机时 V_out 可能还未上升到 1V)
    //   3. 如果 PMBus 离线, 回退到 GPIO 电平判断
    if (PMBus::isDeviceOnline()) {
        if (PMBus::V_out >= 1.0f) {
            return true;
        }
        // PMBus 在线但 V_out 低: 可能是刚开机电压还没建立,
        // 此时以 GPIO 状态为准
        return gpio_get_level(SW_CTRL) == 1;
    }
    // PMBus 离线: 回退到硬件 GPIO 状态
    return gpio_get_level(SW_CTRL) == 1;
}

esp_err_t PowerControl::setVoltage(float voltage)
{
    if (voltage < 0.0f || voltage > PSU_VOLTAGE_MAX) {
        ESP_LOGW(TAG, "Voltage out of range: %.3f (valid: 0~%.1f)", voltage, PSU_VOLTAGE_MAX);
        return ESP_ERR_INVALID_ARG;
    }

    _voltageSet = voltage;

    // 正向逻辑: V_DAC = (V_target / 12.0) * 3.0
    float vDac = (voltage / PSU_VOLTAGE_MAX) * V_DAC_MAX;

    // PWM 占空比 = (V_DAC / 3.3) * 2047
    uint32_t duty = (uint32_t)((vDac / MCU_VDD) * PWM_MAX_DUTY);

    // 钳位到有效范围
    if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;

    _pwmSetDuty(LEDC_CHANNEL_0, duty);
    return ESP_OK;
}

esp_err_t PowerControl::setCurrent(float current)
{
    if (current < 0.0f || current > PSU_CURRENT_MAX) {
        ESP_LOGW(TAG, "Current out of range: %.3f (valid: 0~%.1f)", current, PSU_CURRENT_MAX);
        return ESP_ERR_INVALID_ARG;
    }

    _currentSet = current;

    // 正向逻辑: I_DAC = (I_target / 62.5) * 3.0
    float iDac = (current / PSU_CURRENT_MAX) * V_DAC_MAX;

    // PWM 占空比 = (I_DAC / 3.3) * 8191
    uint32_t duty = (uint32_t)((iDac / MCU_VDD) * PWM_MAX_DUTY);

    if (duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;

    _pwmSetDuty(LEDC_CHANNEL_1, duty);
    return ESP_OK;
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

    // 立即输出 LOW
    gpio_set_level(gpio, 0);
    return ESP_OK;
}

esp_err_t PowerControl::_pwmInit(gpio_num_t gpio, ledc_channel_t channel, uint32_t duty)
{
    // LEDC 定时器配置 (所有 PWM 通道共享一个定时器)
    // 只配置一次, 后续调用忽略
    static bool timerInited = false;
    if (!timerInited) {
        ledc_timer_config_t timerCfg = {};
        timerCfg.speed_mode      = LEDC_LOW_SPEED_MODE;
        timerCfg.timer_num       = LEDC_TIMER_0;
        timerCfg.duty_resolution = (ledc_timer_bit_t)PWM_RES_BITS;
        timerCfg.freq_hz         = PWM_FREQ_HZ;
        timerCfg.clk_cfg         = LEDC_USE_APB_CLK;

        esp_err_t ret = ledc_timer_config(&timerCfg);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "LEDC timer config failed: %d", ret);
            return ret;
        }
        timerInited = true;
    }

    // LEDC 通道配置
    ledc_channel_config_t chanCfg = {};
    chanCfg.speed_mode = LEDC_LOW_SPEED_MODE;
    chanCfg.channel    = channel;
    chanCfg.timer_sel  = LEDC_TIMER_0;
    chanCfg.intr_type  = LEDC_INTR_DISABLE;
    chanCfg.gpio_num   = (int)gpio;
    chanCfg.duty       = duty;
    chanCfg.hpoint     = 0;

    esp_err_t ret = ledc_channel_config(&chanCfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC channel %d config failed", channel);
        return ret;
    }

    return ESP_OK;
}

void PowerControl::_pwmSetDuty(ledc_channel_t channel, uint32_t duty)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}
