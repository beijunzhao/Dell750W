/**
 * power_control.h - 电源控制类
 *
 * 封装开/关机、设定电压、设定电流的接口。
 * 内部隐藏 PWM→DAC 数学转换逻辑。
 *
 * 安全关键:
 *   - 电压控制为反向逻辑: PWM占空比=0 时输出最高电压(12V)!
 *   - 电流控制为正向逻辑: PWM占空比 对应 0~60A
 */
#ifndef POWER_CONTROL_H
#define POWER_CONTROL_H

#include "pin_map.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"

class PowerControl {
public:
    /**
     * 初始化电源控制模块 (仅在 setup 安全时序中调用)
     * 注意: 此函数将 SW_CTRL 设为 LOW(关机), V_PWM 设为最高占空比(3.0V→0V输出), I_PWM 设为0
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
     * @param voltage 目标电压 (0.0 ~ 12.0V)
     * @return ESP_OK 或 ESP_ERR_INVALID_ARG
     *
     * 内部公式(反向逻辑):
     *   V_DAC = 3.0 - (voltage / 12.0) * 3.0
     *   PWM_duty = (V_DAC / 3.3) * PWM_MAX_DUTY
     */
    static esp_err_t setVoltage(float voltage);

    /**
     * 设定目标输出电流
     * @param current 目标电流 (0.0 ~ 60.0A)
     * @return ESP_OK 或 ESP_ERR_INVALID_ARG
     *
     * 内部公式(正向逻辑):
     *   I_DAC = (current / 60.0) * 3.0
     *   PWM_duty = (I_DAC / 3.3) * PWM_MAX_DUTY
     */
    static esp_err_t setCurrent(float current);

    /** 获取当前设定的电压值 */
    static float getSetVoltage() { return _voltageSet; }

    /** 获取当前设定的电流值 */
    static float getSetCurrent() { return _currentSet; }

private:
    static float _voltageSet;
    static float _currentSet;
    static bool  _powerState;

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
