/**
 * adc_sampler.h - ADC 采样类
 *
 * 采样链路: 4x过采样 → 3点移动平均 → 6点插值校准 → EMA 平滑 → 死区滞回
 * 原始ADC值直接传入校准函数，不预先减零点偏移（校准表本身覆盖零点）。
 * 零点偏移仅用于调试参考。
 *
 * 所有共享变量受 _mutex 保护，sample() 整流程持锁。
 */
#ifndef ADC_SAMPLER_H
#define ADC_SAMPLER_H

#include "pin_map.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
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
    static const int MA_WINDOW = 3;                    /* 移动平均窗口大小 */
    static constexpr float EMA_ALPHA = 0.2f;           /* EMA 平滑系数 */

    static adc_oneshot_unit_handle_t _adcHandle;
    static adc_channel_t            _adcChannel;
    static adc_cali_handle_t        _caliHandle;       /* 硬件校准句柄 */

    static float _linearVoltage;        /* 未校准线性估算电压（调试用，经过硬件校准） */
    static float _calibratedVoltage;    /* 校准后电压（供 UI 使用，已EMA平滑） */
    static int   _rawAdcValue;          /* 过采样平均后的原始 ADC */
    static int   _zeroOffset;           /* 零点偏移（仅调试参考） */
    static bool  _initialized;          /* 防重入标志 */
    static SemaphoreHandle_t _mutex;    /* 保护所有共享变量 */
    static bool  _inDeadZone;           /* 死区迟滞标志 */
    static float _emaCalVoltage;        /* EMA 状态值 */
    static bool  _emaInited;            /* EMA 是否已初始化 */

    /* 移动平均滤波缓冲区（受 _mutex 保护） */
    static float _filterBuf[MA_WINDOW];
    static int   _filterIdx;
    static bool  _filterFilled;

    static float _movingAverageFilter(float newValue);
};

#endif // ADC_SAMPLER_H