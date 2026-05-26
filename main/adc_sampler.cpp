/**
 * adc_sampler.cpp - ADCSampler 类实现
 */
#include "adc_sampler.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <cstring>

static const char* TAG = "ADCSampler";
static const char* NVS_NAMESPACE = "adc_cal";
static const char* NVS_KEY_MULT   = "cal_mult";
static const char* NVS_KEY_OFFSET = "cal_off";

// 静态成员初始化
adc_oneshot_unit_handle_t ADCSampler::_adcHandle = nullptr;
adc_channel_t            ADCSampler::_adcChannel = ADC_CHANNEL_0;
float                    ADCSampler::_filteredVoltage = 0.0f;
float                    ADCSampler::_calibratedVoltage = 0.0f;
int                      ADCSampler::_rawAdcValue = 0;
float                    ADCSampler::_calMultiplier = ADC_CAL_MULTIPLIER;
float                    ADCSampler::_calOffset = ADC_CAL_OFFSET;

// 移动平均滤波缓冲区
static float _filterBuf[ADC_SAMPLE_COUNT] = {};
static int   _filterIdx = 0;
static bool  _filterFilled = false;

/* ========== NVS 持久化 ========== */

esp_err_t ADCSampler::loadCalFromNVS()
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "No NVS calibration found, using defaults");
        return ret;
    }

    float mult = _calMultiplier, off = _calOffset;
    size_t sz = sizeof(float);
    ret = nvs_get_blob(handle, NVS_KEY_MULT, &mult, &sz);
    if (ret == ESP_OK) {
        _calMultiplier = mult;
    }
    sz = sizeof(float);
    ret = nvs_get_blob(handle, NVS_KEY_OFFSET, &off, &sz);
    if (ret == ESP_OK) {
        _calOffset = off;
    }

    nvs_close(handle);
    ESP_LOGI(TAG, "Calibration loaded from NVS: mult=%.4f offset=%.4f",
             _calMultiplier, _calOffset);
    return ESP_OK;
}

esp_err_t ADCSampler::saveCalToNVS()
{
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_set_blob(handle, NVS_KEY_MULT, &_calMultiplier, sizeof(float));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS write mult failed");
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_blob(handle, NVS_KEY_OFFSET, &_calOffset, sizeof(float));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS write offset failed");
        nvs_close(handle);
        return ret;
    }
    ret = nvs_commit(handle);
    nvs_close(handle);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration saved to NVS: mult=%.4f offset=%.4f",
                 _calMultiplier, _calOffset);
    }
    return ret;
}

/* ========== 初始化和校准 ========== */

esp_err_t ADCSampler::init()
{
    adc_oneshot_unit_init_cfg_t unitCfg = {};
    unitCfg.unit_id  = ADC_UNIT_1;

    esp_err_t ret = adc_oneshot_new_unit(&unitCfg, &_adcHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC oneshot unit init failed: %d", ret);
        return ret;
    }

    adc_oneshot_chan_cfg_t chanCfg = {};
    chanCfg.atten    = ADC_ATTEN_DB_12;
    chanCfg.bitwidth = ADC_BITWIDTH_DEFAULT;

    ret = adc_oneshot_config_channel(_adcHandle, _adcChannel, &chanCfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC channel config failed: %d", ret);
        return ret;
    }

    memset(_filterBuf, 0, sizeof(_filterBuf));
    _filterIdx = 0;
    _filterFilled = false;

    // 先设默认值, 再尝试从 NVS 加载
    _calMultiplier = ADC_CAL_MULTIPLIER;
    _calOffset = ADC_CAL_OFFSET;
    loadCalFromNVS();

    ESP_LOGI(TAG, "ADC initialized (GPIO0, ADC1_CH0, 12-bit, 0~3.3V)");
    ESP_LOGI(TAG, "Divider ratio=%.3f, cal: mult=%.4f offset=%.4f",
             ADC_DIVIDER_RATIO, _calMultiplier, _calOffset);
    return ESP_OK;
}

void ADCSampler::calibrate(float multiplier, float offset)
{
    _calMultiplier = multiplier;
    _calOffset = offset;
    saveCalToNVS();
    ESP_LOGI(TAG, "Calibration updated: mult=%.4f offset=%.4f", multiplier, offset);
}

/* ========== 采样 ========== */

float ADCSampler::sample()
{
    int rawAdc = 0;

    esp_err_t ret = adc_oneshot_read(_adcHandle, _adcChannel, &rawAdc);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC read failed: %d", ret);
        return _calibratedVoltage;
    }

    _rawAdcValue = rawAdc;

    // 原始电压: ADC → MCU 电压 → 实际电压 (分压还原)
    float adcVoltage = ((float)rawAdc / 4095.0f) * MCU_VDD;
    float realVoltage = adcVoltage * ADC_DIVIDER_RATIO;

    // 移动平均滤波
    _filteredVoltage = _movingAverageFilter(realVoltage);

    // 校准: V_cal = V_raw * mult + offset
    _calibratedVoltage = _filteredVoltage * _calMultiplier + _calOffset;

    return _calibratedVoltage;
}

/* ========== 移动平均滤波 ========== */

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
