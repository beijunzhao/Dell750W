/**
 * ble_server.h - BLE GATT 服务器 (Nordic UART Service)
 *
 * 与 uni-app 安卓 App 通过 JSON over BLE UART 通信。
 *
 * Service UUID:  6E400001-B5A3-F393-E0A9-E50E24DCCA9E
 * TX Char (Notify): 6E400002-...  (ESP32 → App)
 * RX Char (Write):  6E400003-...  (App → ESP32)
 */
#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include "pin_map.h"
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化 BLE GATT 服务器并开始广播
 * 注意: 调用前需确保 NVS 和 NimBLE 已初始化
 */
esp_err_t ble_server_init(void);

/**
 * 通过 BLE 通知发送数据给已连接的客户端
 * @param data 要发送的字符串
 * @return ESP_OK 或错误码
 */
esp_err_t ble_server_send(const char* data);

/** 检查是否有客户端已连接 */
bool ble_server_is_connected(void);

/**
 * 设置接收数据回调
 * @param cb 回调函数 (data, len)
 */
typedef void (*ble_rx_callback_t)(const char* data, int len);
void ble_server_set_rx_callback(ble_rx_callback_t cb);

#ifdef __cplusplus
}
#endif

#endif // BLE_SERVER_H
