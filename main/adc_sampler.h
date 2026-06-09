/**
 * adc_sampler.h - ADC 采样类
 *
 * 采样链路: 4x过采样 → 3点移动平均 → 6点插值校准
 * 原始ADC值直接传入校准函数。
 *
 * 所有共享变量受 _mutex 保护，sample() 整流程持锁。
 */
#ifndef ADC_SAMPLER_H
#define ADC_SAMPLER_H

#include "pin_map.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class ADCSampler {
public:
    static esp_err_t init();
    static float sample();
    static float getVoltage();
    static float getRawVoltage();
    static int getRawAdc();
    static int getZeroOffset();
    static void zeroCalibrate();
    static esp_err_t loadCalFromNVS();
    static esp_err_t saveCalToNVS();

private:
    static const int MA_WINDOW = 3;
    static constexpr float EMA_ALPHA = 0.2f;  /* EMA 平滑系数 */

    static adc_oneshot_unit_handle_t _adcHandle;
    static adc_channel_t            _adcChannel;
    static float                    _linearVoltage;
    static float                    _calibratedVoltage;
    static int                      _rawAdcValue;
    static int                      _zeroOffset;
    static bool                     _initialized;
    static SemaphoreHandle_t        _mutex;
    static bool                     _inDeadZone;

    /* EMA 平滑滤波 */
    static float _emaCalVoltage;   /* EMA 输出的校准电压 */
    static bool  _emaInited;       /* EMA 初始化标志 */

    /* 移动平均滤波缓冲区 */
    static float _filterBuf[MA_WINDOW];
    static int   _filterIdx;
    static bool  _filterFilled;

    static float _movingAverageFilter(float newValue);
};

#endif // ADC_SAMPLER_H