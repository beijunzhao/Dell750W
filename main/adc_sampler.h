/**
 * adc_sampler.h - ADC 采样类
 *
 * 封装 ADC 读取、分压还原计算和移动平均滤波算法。
 * 采样 GPIO0, 外部 0~12V 经 20.47k/5k 分压后进入 ADC。
 */
#ifndef ADC_SAMPLER_H
#define ADC_SAMPLER_H

#include "pin_map.h"
#include "esp_adc/adc_oneshot.h"
#include "hal/adc_types.h"
#include "esp_err.h"

class ADCSampler {
public:
    /**
     * 初始化 ADC 单元
     * @return ESP_OK 或错误码
     */
    static esp_err_t init();

    /**
     * 执行一次电压采样 (含 10 次移动平均滤波)
     * @return 还原后的实际电压值 (0 ~ 12V)
     */
    static float sample();

    /**
     * 获取滤波后的最新电压值 (不触发新采样)
     */
    static float getVoltage() { return _filteredVoltage; }

    /**
     * 获取原始 ADC 读数 (调试用)
     */
    static int getRawAdc() { return _rawAdcValue; }

private:
    static adc_oneshot_unit_handle_t _adcHandle;
    static adc_channel_t            _adcChannel;
    static float                    _filteredVoltage;
    static int                      _rawAdcValue;

    /** 移动平均滤波: 加入新值,返回滤波结果 */
    static float _movingAverageFilter(float newValue);
};

#endif // ADC_SAMPLER_H
