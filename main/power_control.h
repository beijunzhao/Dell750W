/**
 * power_control.h - 电源控制类
 *
 * 封装开/关机、设定电压、设定电流的接口。
 * 内部隐藏 PWM→DAC 数学转换逻辑。
 *
 * 安全关键:
 *   - 电压控制为正向逻辑: PWM占空比=0 时输出 0V, PWM占空比~90.9% 时输出 12V
 *   - 电流控制为正向逻辑: PWM占空比 对应 0~62.5A
 */
#ifndef POWER_CONTROL_H
#define POWER_CONTROL_H

#include "pin_map.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class PowerControl {
public:
    static esp_err_t init();
    static void saveToNVS();

    static void powerOn();
    static void powerOff();
    static bool isPoweredOn();

    static esp_err_t setVoltage(float voltage);
    static esp_err_t setCurrent(float current);

    static float getSetVoltage();
    static float getSetCurrent();
    static float getVMax();
    static float getIMax();

    static void setVMax(float vMax);
    static void setIMax(float iMax);

    /** 直接设置 PWM 占空比 (校准模块使用，调用前需确保已初始化) */
    static void setPwmDuty(ledc_channel_t channel, uint32_t duty);

private:
    static float _voltageSet;
    static float _currentSet;
    static bool  _powerState;
    static float _vMax;
    static float _iMax;

    static esp_err_t _gpioInitLow(gpio_num_t gpio);
    static esp_err_t _pwmInit(gpio_num_t gpio, ledc_channel_t channel, uint32_t duty);
    static void _pwmSetDuty(ledc_channel_t channel, uint32_t duty);
};

#endif