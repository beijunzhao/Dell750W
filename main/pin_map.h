/**
 * pin_map.h - 台达/戴尔 750W 数控电源 GPIO 引脚映射表
 * 硬件平台: ESP32-C3 (QFN32)
 */
#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "driver/gpio.h"

// ---------- 模拟/数字输入 ----------
#define V_ADC_IO0       GPIO_NUM_0    // 外部 12V 降压采样 (ADC)
#define BTN_UP          GPIO_NUM_18   // 按键上 (需内部上拉, 低电平有效)
#define BTN_DOWN        GPIO_NUM_19   // 按键下 (需内部上拉, 低电平有效)
#define BTN_OK          GPIO_NUM_20   // 按键确认 (需内部上拉, 低电平有效)

// ---------- 数字输出 ----------
#define SW_CTRL         GPIO_NUM_1    // 电源开关 (HIGH=开机, LOW=关机)

// ---------- PWM 输出 (LEDC) ----------
#define V_PWM           GPIO_NUM_2    // 电压控制 PWM
#define I_PWM           GPIO_NUM_3    // 电流控制 PWM

// ---------- I2C (PMBus) ----------
#define PMBUS_SCL       GPIO_NUM_4    // I2C 时钟
#define PMBUS_SDA       GPIO_NUM_5    // I2C 数据

// ---------- SPI (TFT 屏幕) ----------
#define TFT_SCLK        GPIO_NUM_6    // SPI 时钟
#define TFT_MOSI        GPIO_NUM_7    // SPI 数据 (MOSI)
#define TFT_RST         GPIO_NUM_8    // 屏幕硬件复位
#define TFT_DC          GPIO_NUM_9    // 数据/命令选择 (注意: BOOT 脚, 开机需保持高)
#define TFT_CS          GPIO_NUM_10   // 屏幕片选

// ---------- PWM 硬件参数 ----------
#define MCU_VDD         3.3f          // 单片机逻辑电压
#define V_DAC_MAX       3.0f          // DAC 控制电压上限
#define PWM_FREQ_HZ     20000         // PWM 频率 20kHz
#define PWM_RES_BITS    11            // PWM 分辨率 11-bit (ESP32-C3: 80MHz APB_CLK, 11bit+20kHz 可满足)
#define PWM_MAX_DUTY    2047          // 2^11 - 1 = 2047
#define PWM_DUTY_3V0    1861          // (3.0/3.3)*2047 ≈ 1861 (约90.9%占空比)

// ---------- 电源参数 ----------
#define PSU_VOLTAGE_MAX   12.0f       // 电源最高输出电压
#define PSU_CURRENT_MAX   62.5f       // 电源最高输出电流 (750W / 12V)
#define ADC_DIVIDER_RATIO 5.094f      // 分压比: (20.47+5)/5
#define ADC_SAMPLE_COUNT  10          // 移动平均滤波采样数

// ---------- PMBus ----------
#define PMBUS_I2C_ADDR    0x58        // PMBus I2C 地址
#define PMBUS_I2C_FREQ    100000      // PMBus I2C 频率

// ---------- BLE ----------
#define BLE_DEVICE_NAME   "Dell-PSU-Controller"

#endif // PIN_MAP_H
