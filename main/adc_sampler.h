/**
 * adc_sampler.h - ADC 采样类
 *
 * 封装 ADC 读取、分压还原计算、移动平均滤波和电压校准。
 * 采样 GPIO0, 外部 VOUT 经 470Ω+10k+10k / 5.1k 分压后进入 ADC。
 */
#ifndef ADC_SAMPLER_H
#define ADC_SAMPLER_H

#include "pin_map.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_err.h"

class ADCSampler {
public:
    /** 初始化 ADC 单元 */
    static esp_err_t init();

    /** 执行一次电压采样 (含 10 次移动平均滤波 + 校准) */
    static float sample();

    /** 获取校准后的电压值 (不触发新采样) */
    static float getVoltage() { return _calibratedVoltage; }

    /** 获取未校准的原始电压 (调试/校准用) */
    static float getRawVoltage() { return _filteredVoltage; }

    /** 获取原始 ADC 读数 (调试用) */
    static int getRawAdc() { return _rawAdcValue; }

    /**
     * @brief 设置校准参数 (自动写入 NVS 持久化)
     * @param multiplier 乘数修正 (默认 1.0, 修正分压比误差)
     * @param offset     偏置修正 (默认 0.0V, 修正 ADC 零点漂移)
     */
    static void calibrate(float multiplier, float offset);

    /** 获取校准乘数 */
    static float getCalMultiplier() { return _calMultiplier; }

    /** 获取校准偏置 */
    static float getCalOffset() { return _calOffset; }

    /** 从 NVS 加载校准参数 (init 时自动调用) */
    static esp_err_t loadCalFromNVS();

    /** 保存校准参数到 NVS */
    static esp_err_t saveCalToNVS();

private:
    static adc_oneshot_unit_handle_t _adcHandle;
    static adc_channel_t            _adcChannel;
    static float                    _filteredVoltage;
    static float                    _calibratedVoltage;
    static int                      _rawAdcValue;
    static float                    _calMultiplier;
    static float                    _calOffset;

    /** 移动平均滤波: 加入新值,返回滤波结果 */
    static float _movingAverageFilter(float newValue);
};

#endif // ADC_SAMPLER_H
