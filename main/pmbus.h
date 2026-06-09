/**
 * pmbus.h - PMBus (I2C) 通信类
 *
 * 从 Arduino Wire 参考实现移植到 ESP-IDF I2C Master Driver。
 * 用于读取台达/戴尔服务器电源的内部寄存器数据。
 *
 * 参考: Tomosawa 的 PMBus 库 (通讯1.md/通信2.md)
 */
#ifndef PMBUS_H
#define PMBUS_H

#include "pin_map.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdint.h>

// ---------- PMBus 命令定义 ----------
// 基本信息
#define PMBUS_MFR_ID            0x99
#define PMBUS_MFR_MODEL         0x9A
#define PMBUS_MFR_REVISION      0x9B
#define PMBUS_MFR_LOCATION      0x9C
#define PMBUS_MFR_DATE          0x9D
#define PMBUS_MFR_SERIAL        0x9E
// 规格信息
#define PMBUS_MFR_VIN_MIN       0xA0
#define PMBUS_MFR_VIN_MAX       0xA1
#define PMBUS_MFR_IIN_MAX       0xA2
#define PMBUS_MFR_PIN_MAX       0xA3
#define PMBUS_MFR_VOUT_MIN      0xA4
#define PMBUS_MFR_VOUT_MAX      0xA5
#define PMBUS_MFR_IOUT_MAX      0xA6
#define PMBUS_MFR_POUT_MAX      0xA7
// 输入输出
#define PMBUS_READ_EIN          0x86
#define PMBUS_READ_EOUT         0x87
#define PMBUS_READ_VIN          0x88
#define PMBUS_READ_IIN          0x89
#define PMBUS_READ_VCAP         0x8A
#define PMBUS_READ_VOUT         0x8B
#define PMBUS_READ_IOUT         0x8C
#define PMBUS_READ_POUT         0x96
#define PMBUS_READ_PIN          0x97
// 温度/风扇
#define PMBUS_READ_TEMPERATURE_1 0x8D
#define PMBUS_READ_TEMPERATURE_2 0x8E
#define PMBUS_READ_TEMPERATURE_3 0x8F
#define PMBUS_READ_FAN_SPEED_1   0x90
// 参数设置
#define PMBUS_VOUT_COMMAND      0x21
#define PMBUS_VOUT_MAX          0x24
#define PMBUS_FAN_COMMAND_1     0x3B
#define PMBUS_VOUT_OV_FAULT_LIMIT 0x40
#define PMBUS_IOUT_OC_FAULT_LIMIT 0x46
#define PMBUS_OT_FAULT_LIMIT    0x4F
#define PMBUS_COEFFICIENTS      0x30
#define PMBUS_VOUT_MODE         0x20
#define PMBUS_WRITE_PROTECT     0x10

class PMBus {
public:
    /** 初始化 PMBus I2C 通信 */
    static esp_err_t init();

    /** 检查设备是否在线 (I2C ACK) */
    static bool isDeviceOnline();

    /** 执行一次数据扫描 (读取所有遥测寄存器) */
    static int scan();

    /** 获取 PSU 数据 JSON 字符串 (与 App 协议一致) */
    static const char* getDataJson();

    /** 获取设备信息 JSON */
    static const char* getInfoJson();

    /** 清除故障状态 */
    static void clearFaults();

    /** 检查电源是否在运行 (V_out > 1V) */
    static bool isRunning();

    /**
     * @brief 电流校准点结构体
     */
    typedef struct {
        float target;   /**< 目标电流 (A) */
        float pwm_val;  /**< 确认时的 PWM 值 */
        float raw_val;  /**< 确认时的 PMBus 原始电流 (A) */
    } i_calib_point_t;

    /** 电流校准点数 */
    static const int I_CALIB_POINTS = 6;

    /** 目标电流数组 */
    static const float I_CALIB_TARGETS[I_CALIB_POINTS];

    /**
     * @brief 设置电流校准表 (自动写入 NVS 持久化)
     * @param points 6 个校准点数据
     */
    static void setCurrentCalTable(const i_calib_point_t* points);

    /** 获取电流校准表指针 */
    static const i_calib_point_t* getCurrentCalTable() { return _iCalTable; }

    /** 从 NVS 加载电流校准表 (init 时自动调用) */
    static esp_err_t loadICalTableFromNVS();

    /** 保存电流校准表到 NVS */
    static esp_err_t saveICalTableToNVS();

    /**
     * @brief 应用电流校准表进行插值
     * @param rawI PMBus 原始电流值
     * @return 校准后的电流值
     */
    static float applyICalTable(float rawI);

    // ---------- 公开的遥测数据 ----------
    static float V_in, I_in, V_out, I_out, W_in, W_out, E_in, E_out;
    static float temperature[3], fanSpeed[2];
    static char  mfrId[32], mfrModel[32], mfrRevision[16];
    static char  mfrLocation[32], mfrDate[16], mfrSerial[32];

private:
    static i2c_master_bus_handle_t _busHandle;
    static i2c_master_dev_handle_t _devHandle;
    static bool _bReadMFR;

    // ---------- I2C 底层操作 ----------
    static esp_err_t _writeByte(uint8_t reg, uint8_t value);
    static esp_err_t _readByte(uint8_t reg, uint8_t* value);
    static esp_err_t _readWord(uint8_t reg, uint16_t* value);
    static esp_err_t _readBlock(uint8_t reg, uint8_t* buf, int len);

    // ---------- PMBus 数据解析 ----------
    static float _linear11ToFloat(uint16_t data);
    static float _ulinear16ToFloat(uint16_t data);
    static float _readLinear(uint8_t reg);
    static int8_t _readVoutMode();
    static void  _readCoefficients(uint16_t* m, uint16_t* b, int8_t* R);
    static void  _readMFR();
    static uint16_t _convertHex2Dec(uint16_t hexData);
    /** 电流校准表 (6 点) */
    static i_calib_point_t _iCalTable[I_CALIB_POINTS];
    /** 互斥锁：保护所有静态遥测数据成员 */
    static SemaphoreHandle_t _dataMutex;
};

#endif // PMBUS_H
