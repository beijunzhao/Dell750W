/**
 * adc_sampler.cpp - ADCSampler 类实现
 *
 * 采样链路: 4x过采样 → 3点移动平均 → 6点插值校准 → EMA 平滑 → 死区滞回
 * 原始ADC值直接传入校准函数，不预先减零点偏移（校准表本身覆盖零点）。
 *
 * 线程安全：所有共享变量受 _mutex 保护，
 * sample() 整个流程（过采样→滤波→校准→死区→写变量）都在锁内执行。
 * getter 函数和 zeroCalibrate() 同样持锁。
 */
#include "adc_sampler.h"
#include "calibration.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <cstring>

static const char* TAG = "ADCSampler";
static const char* NVS_NS  = "adc_cal";
static const char* NVS_KEY = "zero_off";

/* 死区迟滞阈值 */
#define DEADZONE_ENTER  0.03f
#define DEADZONE_EXIT   0.07f

/* ========== 静态成员初始化 ========== */
adc_oneshot_unit_handle_t ADCSampler::_adcHandle = nullptr;
adc_channel_t            ADCSampler::_adcChannel = ADC_CHANNEL_0;
adc_cali_handle_t        ADCSampler::_caliHandle = nullptr;
float ADCSampler::_linearVoltage = 0.0f;
float ADCSampler::_calibratedVoltage = 0.0f;
int   ADCSampler::_rawAdcValue = 0;
int   ADCSampler::_zeroOffset = 0;
bool  ADCSampler::_initialized = false;
SemaphoreHandle_t ADCSampler::_mutex = nullptr;
bool  ADCSampler::_inDeadZone = false;
float ADCSampler::_emaCalVoltage = 0.0f;
bool  ADCSampler::_emaInited = false;
float ADCSampler::_filterBuf[ADCSampler::MA_WINDOW] = {};
int   ADCSampler::_filterIdx = 0;
bool  ADCSampler::_filterFilled = false;

/* ========== NVS 持久化 ========== */

esp_err_t ADCSampler::loadCalFromNVS()
{
    int offset = 0;
    nvs_handle_t h;
    esp_err_t ret = nvs_open(NVS_NS, NVS_READONLY, &h);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "NVS open failed (%d), zeroOffset=0", ret);
    } else {
        size_t sz = sizeof(int);
        ret = nvs_get_blob(h, NVS_KEY, &offset, &sz);
        nvs_close(h);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "No zero cal in NVS, set to 0. Run zeroCalibrate()");
            offset = 0;
        } else {
            ESP_LOGI(TAG, "Zero offset loaded: %d", offset);
        }
    }

    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
        _zeroOffset = offset;
        xSemaphoreGive(_mutex);
    }
    return ESP_OK;
}

esp_err_t ADCSampler::saveCalToNVS()
{
    int offset;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
        offset = _zeroOffset;
        xSemaphoreGive(_mutex);
    } else {
        offset = 0;
    }

    nvs_handle_t h;
    esp_err_t ret = nvs_open(NVS_NS, NVS_READWRITE, &h);
    if (ret != ESP_OK) return ret;
    nvs_set_blob(h, NVS_KEY, &offset, sizeof(int));
    nvs_commit(h);
    nvs_close(h);
    ESP_LOGI(TAG, "Zero offset saved: %d", offset);
    return ESP_OK;
}

/* ========== 初始化 ========== */

esp_err_t ADCSampler::init()
{
    if (_initialized) {
        ESP_LOGW(TAG, "Already initialized, skipping");
        return ESP_OK;
    }

    _mutex = xSemaphoreCreateMutex();
    if (!_mutex) { ESP_LOGE(TAG, "Mutex create failed"); return ESP_ERR_NO_MEM; }

    adc_oneshot_unit_init_cfg_t unitCfg = {};
    unitCfg.unit_id = ADC_UNIT_1;
    esp_err_t ret = adc_oneshot_new_unit(&unitCfg, &_adcHandle);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "ADC unit init failed"); return ret; }

    adc_oneshot_chan_cfg_t chanCfg = {};
    chanCfg.atten    = ADC_ATTEN_DB_12;
    chanCfg.bitwidth = ADC_BITWIDTH_DEFAULT;
    ret = adc_oneshot_config_channel(_adcHandle, _adcChannel, &chanCfg);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "ADC channel config failed"); return ret; }

    /* 创建硬件校准句柄 */
    adc_cali_curve_fitting_config_t caliCfg = {};
    caliCfg.unit_id  = ADC_UNIT_1;
    caliCfg.atten    = ADC_ATTEN_DB_12;
    caliCfg.bitwidth = ADC_BITWIDTH_DEFAULT;
    ret = adc_cali_create_scheme_curve_fitting(&caliCfg, &_caliHandle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC hw calibration scheme not available, fallback to linear");
        _caliHandle = nullptr;
    } else {
        ESP_LOGI(TAG, "ADC hardware calibration enabled");
    }

    /* 滤波缓冲区初始化 */
    _filterIdx = 0;
    _filterFilled = false;
    for (int i = 0; i < MA_WINDOW; i++) _filterBuf[i] = 0.0f;
    _inDeadZone = false;
    _emaInited = false;

    /* 加载零点偏移 */
    _zeroOffset = 0;
    loadCalFromNVS();

    /* 预填充移动平均窗口 */
    for (int i = 0; i < MA_WINDOW; i++) {
        int raw = 0;
        if (adc_oneshot_read(_adcHandle, _adcChannel, &raw) == ESP_OK) {
            _movingAverageFilter((float)raw);
        }
        esp_rom_delay_us(100);
    }

    _initialized = true;
    ESP_LOGI(TAG, "ADC initialized (4xOS+%d-MA+EMA+6pt cal, hw cal %s)",
             MA_WINDOW, _caliHandle ? "ON" : "OFF");
    return ESP_OK;
}

void ADCSampler::zeroCalibrate()
{
    int sum = 0, valid = 0;
    for (int i = 0; i < 20; i++) {
        int raw = 0;
        if (adc_oneshot_read(_adcHandle, _adcChannel, &raw) == ESP_OK) {
            sum += raw; valid++;
        }
        esp_rom_delay_us(1000);
    }
    int offset = (valid > 0) ? (sum / valid) : 0;

    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
        _zeroOffset = offset;
        xSemaphoreGive(_mutex);
    }
    saveCalToNVS();
    ESP_LOGI(TAG, "ADC zero calibrated: offset=%d (valid=%d/20)", offset, valid);
}

/* ========== 采样核心（整流程持锁） ========== */

float ADCSampler::sample()
{
    if (!_initialized) return 0.0f;

    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) {
        return _calibratedVoltage;
    }

    /* ① 4x 过采样取平均 */
    int sum = 0, valid = 0;
    for (int i = 0; i < 4; i++) {
        int raw = 0;
        if (adc_oneshot_read(_adcHandle, _adcChannel, &raw) == ESP_OK) {
            sum += raw; valid++;
        }
        esp_rom_delay_us(50);
    }
    if (valid == 0) {
        float v = _calibratedVoltage;
        xSemaphoreGive(_mutex);
        return v;
    }
    int rawAdc = sum / valid;

    /* ② 3点移动平均平滑 */
    float smoothedAdc = _movingAverageFilter((float)rawAdc);

    /* ③ 原始ADC直接传入6点插值校准 */
    float calV = calculate_calibrated_value(smoothedAdc, true);

    /* ④ EMA 平滑 + 死区处理 */
    /* 先处理死区状态切换 */
    if (_inDeadZone) {
        if (calV >= DEADZONE_EXIT) {
            _inDeadZone = false;
            // 退出死区时重置 EMA 为当前值，快速恢复
            _emaCalVoltage = calV;
            _emaInited = true;
        } else {
            calV = 0.0f;
            _emaCalVoltage = 0.0f;  // 保持在零
        }
    } else {
        if (calV < DEADZONE_ENTER) {
            _inDeadZone = true;
            calV = 0.0f;
            _emaCalVoltage = 0.0f;
        }
    }

    /* 正常区域进行 EMA 平滑 */
    if (!_inDeadZone && calV > 0.0f) {
        if (!_emaInited) {
            _emaCalVoltage = calV;
            _emaInited = true;
        } else {
            _emaCalVoltage = EMA_ALPHA * calV + (1.0f - EMA_ALPHA) * _emaCalVoltage;
        }
    } else if (_inDeadZone) {
        _emaCalVoltage = 0.0f;
        _emaInited = false;  // 下次退出死区重新初始化
    }

    /* ⑤ 更新共享变量 */
    _rawAdcValue = rawAdc;
    _calibratedVoltage = _emaCalVoltage;   // 使用 EMA 平滑后的值

    /* ⑥ 计算调试线性电压（启用硬件校准） */
    float linear = 0.0f;
    if (_caliHandle) {
        int voltage_mV = 0;
        esp_err_t calRet = adc_cali_raw_to_voltage(_caliHandle, rawAdc, &voltage_mV);
        if (calRet == ESP_OK) {
            linear = voltage_mV / 1000.0f * ADC_DIVIDER_RATIO;
        } else {
            // 硬件校准失败，回退到简单线性公式
            linear = (rawAdc / 4095.0f) * MCU_VDD * ADC_DIVIDER_RATIO;
        }
    } else {
        linear = (rawAdc / 4095.0f) * MCU_VDD * ADC_DIVIDER_RATIO;
    }
    _linearVoltage = linear;

    xSemaphoreGive(_mutex);
    return _calibratedVoltage;
}

/* ========== getter（全部持锁 + 移入 .cpp） ========== */

float ADCSampler::getVoltage()
{
    float v = 0.0f;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
        v = _calibratedVoltage;
        xSemaphoreGive(_mutex);
    }
    return v;
}

float ADCSampler::getRawVoltage()
{
    float v = 0.0f;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
        v = _linearVoltage;
        xSemaphoreGive(_mutex);
    }
    return v;
}

int ADCSampler::getRawAdc()
{
    int v = 0;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
        v = _rawAdcValue;
        xSemaphoreGive(_mutex);
    }
    return v;
}

int ADCSampler::getZeroOffset()
{
    int v = 0;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
        v = _zeroOffset;
        xSemaphoreGive(_mutex);
    }
    return v;
}

/* ========== 3点移动平均滤波（仅在持锁状态下调用） ========== */

float ADCSampler::_movingAverageFilter(float newValue)
{
    _filterBuf[_filterIdx] = newValue;
    _filterIdx = (_filterIdx + 1) % ADCSampler::MA_WINDOW;
    if (_filterIdx == 0) _filterFilled = true;

    int count = _filterFilled ? ADCSampler::MA_WINDOW : _filterIdx;
    if (count == 0) count = 1;

    float s = 0.0f;
    for (int i = 0; i < count; i++) s += _filterBuf[i];
    return s / (float)count;
}