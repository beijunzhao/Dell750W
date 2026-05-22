/**
 * adc_sampler.cpp - ADCSampler 类实现
 */
#include "adc_sampler.h"
#include "esp_log.h"
#include <cstring>

static const char* TAG = "ADCSampler";

// 静态成员初始化
adc_oneshot_unit_handle_t ADCSampler::_adcHandle = nullptr;
adc_channel_t            ADCSampler::_adcChannel = ADC_CHANNEL_0;
float                    ADCSampler::_filteredVoltage = 0.0f;
int                      ADCSampler::_rawAdcValue = 0;

// 移动平均滤波缓冲区
static float _filterBuf[ADC_SAMPLE_COUNT] = {};
static int   _filterIdx = 0;
static bool  _filterFilled = false;

esp_err_t ADCSampler::init()
{
    // ADC1 初始化 (ESP32-C3 的 GPIO0 → ADC1_CH0)
    adc_oneshot_unit_init_cfg_t unitCfg = {};
    unitCfg.unit_id  = ADC_UNIT_1;

    esp_err_t ret = adc_oneshot_new_unit(&unitCfg, &_adcHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC oneshot unit init failed: %d", ret);
        return ret;
    }

    // 配置 ADC 通道 (GPIO0 → ADC1_CHANNEL_0)
    adc_oneshot_chan_cfg_t chanCfg = {};
    chanCfg.atten    = ADC_ATTEN_DB_12;      // 0~3.3V 量程
    chanCfg.bitwidth = ADC_BITWIDTH_DEFAULT; // 默认位宽 (ESP32-C3: 12-bit)

    ret = adc_oneshot_config_channel(_adcHandle, _adcChannel, &chanCfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC channel config failed: %d", ret);
        return ret;
    }

    // 清空滤波缓冲区
    memset(_filterBuf, 0, sizeof(_filterBuf));
    _filterIdx = 0;
    _filterFilled = false;

    ESP_LOGI(TAG, "ADC initialized (GPIO0, ADC1_CH0, 12-bit, 0~3.3V)");
    return ESP_OK;
}

float ADCSampler::sample()
{
    int rawAdc = 0;

    // 读取 ADC 原始值
    esp_err_t ret = adc_oneshot_read(_adcHandle, _adcChannel, &rawAdc);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC read failed: %d", ret);
        return _filteredVoltage;
    }

    _rawAdcValue = rawAdc;

    // 转换为电压: (raw / 4095) * 3.3V → 再 × 分压比还原实际电压
    float adcVoltage = ((float)rawAdc / 4095.0f) * MCU_VDD;
    float realVoltage = adcVoltage * ADC_DIVIDER_RATIO;

    // 移动平均滤波
    _filteredVoltage = _movingAverageFilter(realVoltage);

    return _filteredVoltage;
}

float ADCSampler::_movingAverageFilter(float newValue)
{
    _filterBuf[_filterIdx] = newValue;
    _filterIdx = (_filterIdx + 1) % ADC_SAMPLE_COUNT;

    if (_filterIdx == 0) {
        _filterFilled = true;
    }

    int count = _filterFilled ? ADC_SAMPLE_COUNT : _filterIdx;
    if (count == 0) count = 1;

    float sum = 0.0f;
    for (int i = 0; i < count; i++) {
        sum += _filterBuf[i];
    }

    return sum / (float)count;
}
