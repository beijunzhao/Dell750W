/**
 * pmbus.cpp - PMBus 类实现
 *
 * 从 Arduino Wire 参考实现移植到 ESP-IDF I2C Master Driver。
 * 关键设计决策:
 *   1. 不使用 i2c_master_probe() — 该函数发送的地址探测包可能触发 PSU 安全锁死
 *   2. 使用 i2c_master_transmit_receive() 实现 repeated START,
 *      匹配 Arduino Wire 的 beginTransmission + endTransmission(false) + requestFrom 模式
 *   3. 每个寄存器读取间加 1ms 延时, 匹配参考实现
 *   4. scan() 内部有 1 秒间隔限制, 避免频繁访问
 *
 * 参考: Tomosawa MYNOVA_POWER (https://github.com/Tomosawa/MYNOVA-SmartPower)
 */
#include "pmbus.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include <cmath>
#include <cstdio>
#include <cstring>

static const char* TAG = "PMBus";

// I2C 超时 (ms), PMBus 设备可能有时钟拉伸, 50ms 匹配 Arduino Wire 库默认值
static const int I2C_TIMEOUT_MS = 50;

// 静态成员初始化
i2c_master_bus_handle_t PMBus::_busHandle = nullptr;
i2c_master_dev_handle_t PMBus::_devHandle = nullptr;
bool PMBus::_bReadMFR = false;

float PMBus::V_in = 0, PMBus::I_in = 0, PMBus::V_out = 0, PMBus::I_out = 0;
float PMBus::W_in = 0, PMBus::W_out = 0, PMBus::E_in = 0, PMBus::E_out = 0;
float PMBus::temperature[3] = {}, PMBus::fanSpeed[2] = {};
char  PMBus::mfrId[32] = {}, PMBus::mfrModel[32] = {}, PMBus::mfrRevision[16] = {};
char  PMBus::mfrLocation[32] = {}, PMBus::mfrDate[16] = {}, PMBus::mfrSerial[32] = {};

// 全局 JSON buffer (避免栈上分配大buffer)
static char _jsonBuf[1024];

// VOUT 格式相关
static int8_t  _exponent = -1;
static bool    _isVOutLinear = true;
static uint16_t _coeffM = 0, _coeffB = 0;
static int8_t  _coeffR = 0;

esp_err_t PMBus::init()
{
    // 配置 I2C 主机总线
    i2c_master_bus_config_t busCfg = {};
    busCfg.clk_source    = I2C_CLK_SRC_DEFAULT;
    busCfg.i2c_port      = I2C_NUM_0;
    busCfg.scl_io_num    = PMBUS_SCL;
    busCfg.sda_io_num    = PMBUS_SDA;
    busCfg.glitch_ignore_cnt = 7;
    // 启用内部上拉电阻, 防止外部上拉缺失导致 I2C 总线锁死
    busCfg.flags.enable_internal_pullup = true;

    esp_err_t ret = i2c_new_master_bus(&busCfg, &_busHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C bus init failed: %d", ret);
        return ret;
    }

    // 添加 PMBus 设备
    i2c_device_config_t devCfg = {};
    devCfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    devCfg.device_address   = PMBUS_I2C_ADDR;
    devCfg.scl_speed_hz     = PMBUS_I2C_FREQ;

    ret = i2c_master_bus_add_device(_busHandle, &devCfg, &_devHandle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C device add failed: %d", ret);
        return ret;
    }

    ESP_LOGI(TAG, "PMBus I2C initialized (addr=0x%02X, freq=%dHz)", PMBUS_I2C_ADDR, PMBUS_I2C_FREQ);

    // 等待 PSU 稳定 (匹配参考实现 init() 中的 delay(500))
    esp_rom_delay_us(500000); // 500ms

    return ESP_OK;
}

/**
 * 检查设备是否在线。
 *
 * 匹配参考实现 (通讯1.md:629-639) 的 checkDeviceOnline() 逻辑:
 *   参考实现使用 I2C->beginTransmission(addr) + I2C->endTransmission() 只发送地址检查 ACK。
 *
 * 这里使用 i2c_master_transmit() 发送 PMBUS_REVISION 寄存器地址 (0x98),
 * 只检查地址 ACK + 寄存器 ACK, 不等待数据响应。
 * 这比 i2c_master_transmit_receive() 更可靠, 因为某些 PSU 在特定状态下
 * 可能不响应寄存器读取 (超时), 但会响应地址探测。
 *
 * 注意: 不使用 i2c_master_probe() — 该函数发送的地址探测包可能触发 PSU 安全锁死。
 * 发送一个已知安全的只读寄存器地址作为替代探测方式。
 */
bool PMBus::isDeviceOnline()
{
    uint8_t reg = 0x98; // PMBUS_REVISION — 所有 PMBus 设备必须支持的只读寄存器
    // 只发送寄存器地址, 不读取数据
    // 相当于参考实现的 beginTransmission + write(reg) + endTransmission(true)
    esp_err_t ret = i2c_master_transmit(_devHandle, &reg, 1, I2C_TIMEOUT_MS);
    return (ret == ESP_OK);
}

int PMBus::scan()
{
    // 1 秒间隔限制, 匹配参考实现
    static uint64_t lastScan = 0;
    uint64_t now = esp_timer_get_time() / 1000; // ms
    if ((now - lastScan) < 1000) return 0;
    lastScan = now;

    // 检查设备在线 (使用 transmit_receive, 不使用 probe)
    if (!isDeviceOnline()) return 0;

    // 首次扫描: 读取厂商信息和 VOUT 格式
    if (!_bReadMFR) {
        _readMFR();
        _readCoefficients(&_coeffM, &_coeffB, &_coeffR);
        _isVOutLinear = (_readVoutMode() >= 0);
        _exponent = -1; // 电源约定 exponent = -1
        _writeByte(PMBUS_WRITE_PROTECT, 0x00); // 解除写保护
    }

    // 读取遥测数据, 每个寄存器间加 1ms 延时 (匹配参考实现)
    E_in  = _readLinear(PMBUS_READ_EIN);
    esp_rom_delay_us(1000);
    E_out = _readLinear(PMBUS_READ_EOUT);
    esp_rom_delay_us(1000);
    V_in  = _readLinear(PMBUS_READ_VIN);
    esp_rom_delay_us(1000);
    I_in  = _readLinear(PMBUS_READ_IIN);
    esp_rom_delay_us(1000);
    W_in  = _readLinear(PMBUS_READ_PIN);
    esp_rom_delay_us(1000);

    uint16_t voutRaw;
    if (_readWord(PMBUS_READ_VOUT, &voutRaw) == ESP_OK) {
        V_out = _ulinear16ToFloat(voutRaw) / 1000.0f;
    }
    esp_rom_delay_us(1000);

    I_out = _readLinear(PMBUS_READ_IOUT);
    esp_rom_delay_us(1000);
    W_out = _readLinear(PMBUS_READ_POUT);
    esp_rom_delay_us(1000);

    temperature[0] = _readLinear(PMBUS_READ_TEMPERATURE_1);
    esp_rom_delay_us(1000);
    temperature[1] = _readLinear(PMBUS_READ_TEMPERATURE_2);
    esp_rom_delay_us(1000);
    temperature[2] = _readLinear(PMBUS_READ_TEMPERATURE_3);
    esp_rom_delay_us(1000);
    fanSpeed[0]    = _readLinear(PMBUS_READ_FAN_SPEED_1);

    ESP_LOGI(TAG, "Telemetry: V_in=%.1fV I_in=%.2fA V_out=%.3fV I_out=%.2fA W_out=%.1fW T=%.1f/%.1f/%.1f Fan=%.0fRPM",
             V_in, I_in, V_out, I_out, W_out,
             temperature[0], temperature[1], temperature[2],
             fanSpeed[0]);

    return 1;
}

const char* PMBus::getDataJson()
{
    snprintf(_jsonBuf, sizeof(_jsonBuf),
        "{"
        "\"V_out\":%.3f,\"I_out\":%.3f,"
        "\"V_in\":%.3f,\"I_in\":%.3f,"
        "\"W_out\":%.1f,\"W_in\":%.1f,"
        "\"E_out\":%.1f,\"E_in\":%.1f,"
        "\"temperature\":[%.1f,%.1f,%.1f],"
        "\"fan_speed\":%.0f,"
        "\"device_online\":true"
        "}",
        V_out, I_out,
        V_in, I_in,
        W_out, W_in,
        E_out, E_in,
        temperature[0], temperature[1], temperature[2],
        fanSpeed[0]
    );
    return _jsonBuf;
}

const char* PMBus::getInfoJson()
{
    snprintf(_jsonBuf, sizeof(_jsonBuf),
        "{"
        "\"MFR_ID\":\"%s\",\"MFR_MODEL\":\"%s\","
        "\"MFR_REVISION\":\"%s\",\"MFR_LOCATION\":\"%s\","
        "\"MFR_DATE\":\"%s\",\"MFR_SERIAL\":\"%s\","
        "\"pmbus_revision\":0"
        "}",
        mfrId, mfrModel,
        mfrRevision, mfrLocation,
        mfrDate, mfrSerial
    );
    return _jsonBuf;
}

void PMBus::clearFaults()
{
    _writeByte(0x03, 0x00);
    ESP_LOGI(TAG, "Faults cleared");
}

bool PMBus::isRunning()
{
    return V_out >= 1.0f;
}

// ==================== I2C 底层操作 ====================
//
// 使用 i2c_master_transmit_receive() 实现 repeated START,
// 匹配 Arduino Wire 的 beginTransmission + endTransmission(false) + requestFrom 模式。
// 这确保在发送寄存器地址后不发送 STOP, 而是直接发送 repeated START 读取数据。

esp_err_t PMBus::_writeByte(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = { reg, value };
    return i2c_master_transmit(_devHandle, buf, 2, I2C_TIMEOUT_MS);
}

esp_err_t PMBus::_readByte(uint8_t reg, uint8_t* value)
{
    // transmit_receive 实现 repeated START: 先发 reg, 再读 1 字节
    return i2c_master_transmit_receive(_devHandle, &reg, 1, value, 1, I2C_TIMEOUT_MS);
}

esp_err_t PMBus::_readWord(uint8_t reg, uint16_t* value)
{
    uint8_t rxBuf[2] = {0, 0};

    // transmit_receive 实现 repeated START: 先发 reg, 再读 2 字节
    esp_err_t ret = i2c_master_transmit_receive(_devHandle, &reg, 1, rxBuf, 2, I2C_TIMEOUT_MS);
    if (ret != ESP_OK) {
        *value = 0;
        return ret;
    }

    *value = ((uint16_t)rxBuf[1] << 8) | rxBuf[0];
    return ESP_OK;
}

esp_err_t PMBus::_readBlock(uint8_t reg, uint8_t* buf, int len)
{
    // transmit_receive 实现 repeated START: 先发 reg, 再读 len 字节
    return i2c_master_transmit_receive(_devHandle, &reg, 1, buf, len, I2C_TIMEOUT_MS);
}

// ==================== PMBus 数据解析 ====================

void PMBus::_readMFR()
{
    uint8_t len;

    // PMBus Block Read 协议:
    //   响应格式为 [ByteCount][Data...]
    //   ByteCount 包含自身, 所以实际数据长度 = ByteCount - 1
    //   参考实现中 read_string() 直接使用 read_byte() 返回的值作为长度
    //   然后 requestFrom(addr, len, 1) 读取 len 字节
    //   注意: 参考实现的 read_byte() 返回的是 ByteCount (包含自身)

    if (_readByte(PMBUS_MFR_ID, &len) == ESP_OK && len > 1) {
        uint8_t data[32] = {};
        // 读取 len 字节 (包含 ByteCount 自身)
        if (_readBlock(PMBUS_MFR_ID, data, len) == ESP_OK) {
            int j = 0;
            // 从索引 1 开始跳过 ByteCount 字节
            for (int i = 1; i < len && j < 31; i++) {
                if (data[i] >= 32 && data[i] <= 126) mfrId[j++] = data[i];
            }
            mfrId[j] = '\0';
        }
    }

    if (_readByte(PMBUS_MFR_MODEL, &len) == ESP_OK && len > 1) {
        uint8_t data[32] = {};
        if (_readBlock(PMBUS_MFR_MODEL, data, len) == ESP_OK) {
            int j = 0;
            for (int i = 1; i < len && j < 31; i++) {
                if (data[i] >= 32 && data[i] <= 126) mfrModel[j++] = data[i];
            }
            mfrModel[j] = '\0';
        }
    }

    if (_readByte(PMBUS_MFR_REVISION, &len) == ESP_OK && len > 1) {
        uint8_t data[16] = {};
        if (_readBlock(PMBUS_MFR_REVISION, data, len) == ESP_OK) {
            int j = 0;
            for (int i = 1; i < len && j < 15; i++) {
                if (data[i] >= 32 && data[i] <= 126) mfrRevision[j++] = data[i];
            }
            mfrRevision[j] = '\0';
        }
    }

    if (_readByte(PMBUS_MFR_LOCATION, &len) == ESP_OK && len > 1) {
        uint8_t data[32] = {};
        if (_readBlock(PMBUS_MFR_LOCATION, data, len) == ESP_OK) {
            int j = 0;
            for (int i = 1; i < len && j < 31; i++) {
                if (data[i] >= 32 && data[i] <= 126) mfrLocation[j++] = data[i];
            }
            mfrLocation[j] = '\0';
        }
    }

    if (_readByte(PMBUS_MFR_DATE, &len) == ESP_OK && len > 1) {
        uint8_t data[16] = {};
        if (_readBlock(PMBUS_MFR_DATE, data, len) == ESP_OK) {
            int j = 0;
            for (int i = 1; i < len && j < 15; i++) {
                if (data[i] >= 32 && data[i] <= 126) mfrDate[j++] = data[i];
            }
            mfrDate[j] = '\0';
        }
    }

    if (_readByte(PMBUS_MFR_SERIAL, &len) == ESP_OK && len > 1) {
        uint8_t data[32] = {};
        if (_readBlock(PMBUS_MFR_SERIAL, data, len) == ESP_OK) {
            int j = 0;
            for (int i = 1; i < len && j < 31; i++) {
                if (data[i] >= 32 && data[i] <= 126) mfrSerial[j++] = data[i];
            }
            mfrSerial[j] = '\0';
        }
    }

    _bReadMFR = true;
    ESP_LOGI(TAG, "MFR: %s %s %s", mfrId, mfrModel, mfrSerial);
}

float PMBus::_linear11ToFloat(uint16_t data)
{
    int16_t exponent11 = (data >> 11) & 0x1F;
    if (exponent11 > 15) exponent11 -= 32;

    int16_t mantissa = data & 0x7FF;
    if (mantissa > 1023) mantissa -= 2048;

    return (float)mantissa * powf(2.0f, (float)exponent11);
}

float PMBus::_ulinear16ToFloat(uint16_t data)
{
    uint16_t decData = _convertHex2Dec(data);
    return (float)decData * powf(2.0f, (float)_exponent);
}

float PMBus::_readLinear(uint8_t reg)
{
    uint16_t raw;

    if (_readWord(reg, &raw) != ESP_OK) {
        return 0.0f;
    }

    float result = _linear11ToFloat(raw);

    // 打印所有 LINEAR11 寄存器的原始值和解析结果 (调试用)
    ESP_LOGD(TAG, "_readLinear(0x%02X): raw=0x%04X, result=%.3f", reg, raw, result);

    // 数据有效性检查
    // 1. 绝对值过大 (硬件错误或总线噪声)
    if (fabsf(result) > 1e9f) {
        ESP_LOGW(TAG, "_readLinear(0x%02X): overflow %.1f (raw=0x%04X)", reg, result, raw);
        return 0.0f;
    }

    // 2. E_in/E_out (0x86/0x87) 是累计电能, 不会为负且不应跳变
    //    如果读回负值或明显异常值, 保留上次值
    if (reg == PMBUS_READ_EIN || reg == PMBUS_READ_EOUT) {
        static float lastE_in = 0, lastE_out = 0;
        float& lastVal = (reg == PMBUS_READ_EIN) ? lastE_in : lastE_out;

        // 电源关闭时 (V_out < 1V), E_in/E_out 可能返回无效值, 保持上次值
        if (V_out < 1.0f) {
            return lastVal;
        }

        // 负值: 累计电能不应为负, 保持上次值
        if (result < 0) {
            ESP_LOGW(TAG, "_readLinear(0x%02X): negative E %.1f (raw=0x%04X), keeping last %.1f",
                     reg, result, raw, lastVal);
            return lastVal;
        }

        // 合理上限检查: 累计电能不应超过 1000 Wh (对于 750W 电源, 满载约 1 小时)
        // 如果读回值超过此上限, 说明数据异常 (如寄存器格式不匹配或总线错误)
        if (result > 1000.0f) {
            ESP_LOGW(TAG, "_readLinear(0x%02X): E %.1f exceeds max 1000 (raw=0x%04X), keeping last %.1f",
                     reg, result, raw, lastVal);
            return lastVal;
        }

        // 跳变检测: 累计电能应平滑增长, 单次跳变超过 100 Wh 视为读取错误
        // E_in/E_out 的单位是 Wh, 正常工作时每秒增长很小
        float delta = fabsf(result - lastVal);
        if (delta > 100.0f) {
            ESP_LOGW(TAG, "_readLinear(0x%02X): E jump %.1f -> %.1f (raw=0x%04X), keeping last %.1f",
                     reg, lastVal, result, raw, lastVal);
            return lastVal;
        }

        lastVal = result;
    }

    return result;
}

int8_t PMBus::_readVoutMode()
{
    uint8_t val;
    if (_readByte(PMBUS_VOUT_MODE, &val) != ESP_OK) return -1;

    int8_t exp = val & 0x1F;
    if (exp > 15) exp -= 32;
    return exp;
}

void PMBus::_readCoefficients(uint16_t* m, uint16_t* b, int8_t* R)
{
    uint8_t tx = PMBUS_COEFFICIENTS;
    uint8_t rx[6] = {};

    // transmit_receive 实现 repeated START
    esp_err_t ret = i2c_master_transmit_receive(_devHandle, &tx, 1, rx, 6, I2C_TIMEOUT_MS);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Read COEFFICIENTS failed: %d", ret);
        return;
    }

    *m = (uint16_t)rx[0] | ((uint16_t)rx[1] << 8);
    *b = (uint16_t)rx[2] | ((uint16_t)rx[3] << 8);
    *R = (int8_t)rx[4];
}

uint16_t PMBus::_convertHex2Dec(uint16_t hexData)
{
    return (hexData / 1000) * 4096
         + ((hexData % 1000) / 100) * 256
         + ((hexData % 100) / 10) * 16
         + (hexData % 10);
}
