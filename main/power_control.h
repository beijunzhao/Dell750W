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
#include "nvs_flash.h"
#include "nvs.h"

class PowerControl {
public:
    /**
     * 初始化电源控制模块 (仅在 setup 安全时序中调用)
     * 注意: 此函数将 SW_CTRL 设为 LOW(关机), V_PWM 设为 0 (0V输出), I_PWM 设为0
     */
    static esp_err_t init();

    /** 开启电源 (SW_CTRL = HIGH) */
    static void powerOn();

    /** 关闭电源 (SW_CTRL = LOW) */
    static void powerOff();

    /** 获取当前开关状态 */
    static bool isPoweredOn();

    /**
     * 设定目标输出电压
     * @param voltage 目标电压 (0.0 ~ _vMax)
     * @return ESP_OK 或 ESP_ERR_INVALID_ARG
     *
     * 内部公式(正向逻辑):
     *   V_DAC = (voltage / _vMax) * 3.0
     *   PWM_duty = (V_DAC / 3.3) * PWM_MAX_DUTY
     */
    static esp_err_t setVoltage(float voltage);

    /**
     * 设定目标输出电流
     * @param current 目标电流 (0.0 ~ _iMax)
     * @return ESP_OK 或 ESP_ERR_INVALID_ARG
     *
     * 内部公式(正向逻辑):
     *   I_DAC = (current / _iMax) * 3.0
     *   PWM_duty = (I_DAC / 3.3) * PWM_MAX_DUTY
     */
    static esp_err_t setCurrent(float current);

    /** 获取当前设定的电压值 */
    static float getSetVoltage() { return _voltageSet; }

    /** 获取当前设定的电流值 */
    static float getSetCurrent() { return _currentSet; }

    /** 获取当前量程上限 (V_MAX) */
    static float getVMax() { return _vMax; }

    /** 获取当前量程上限 (I_MAX) */
    static float getIMax() { return _iMax; }

    /**
     * 设置电压量程上限
     * @param vMax 新的电压上限 (不超过 PSU_VOLTAGE_MAX)
     */
    static void setVMax(float vMax);

    /**
     * 设置电流量程上限
     * @param iMax 新的电流上限 (不超过 PSU_CURRENT_MAX)
     */
    static void setIMax(float iMax);

    /**
     * @brief 直接设置 PWM 占空比 (校准模块使用)
     * @param channel LEDC 通道
     * @param duty    占空比 (0 ~ PWM_MAX_DUTY)
     */
    static void setPwmDuty(ledc_channel_t channel, uint32_t duty);

private:
    static float _voltageSet;
    static float _currentSet;
    static bool  _powerState;
    static float _vMax;   // 当前电压量程上限 (默认 PSU_VOLTAGE_MAX_DEF)
    static float _iMax;   // 当前电流量程上限 (默认 PSU_CURRENT_MAX_DEF)

    /** 将指定 GPIO 初始化为推挽输出并设为 LOW */
    static esp_err_t _gpioInitLow(gpio_num_t gpio);

    /**
     * 初始化 LEDC PWM 通道
     * @param gpio     PWM 输出 GPIO
     * @param channel  LEDC 通道
     * @param duty     初始占空比 (0 ~ PWM_MAX_DUTY)
     */
    static esp_err_t _pwmInit(gpio_num_t gpio, ledc_channel_t channel, uint32_t duty);

    /** 设置 PWM 占空比 */
    static void _pwmSetDuty(ledc_channel_t channel, uint32_t duty);
};

#endif // POWER_CONTROL_H
