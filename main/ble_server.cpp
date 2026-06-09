/**
 * ble_server.cpp - BLE NimBLE GATT 服务器实现
 *
 * Nordic UART Service (NUS) profile
 * 参考: ESP-IDF NimBLE examples
 */
#include "ble_server.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_att.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <cstring>
#include <cstdio>

static const char* TAG = "BLE_Server";

// ---------- UUID 定义 (16-bit 短 UUID 不适用, 使用 128-bit) ----------
// NUS Service: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
static const ble_uuid128_t g_nus_svc_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                     0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E);

// TX Characteristic (Notify): 6E400002-...
static const ble_uuid128_t g_nus_tx_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                     0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E);

// RX Characteristic (Write): 6E400003-...
static const ble_uuid128_t g_nus_rx_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                     0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E);

// ---------- 静态变量 ----------
static uint16_t g_conn_handle   = 0;
static uint16_t g_tx_val_handle = 0;
static bool     g_connected     = false;
static bool     g_synced        = false;
static ble_rx_callback_t g_rx_callback = nullptr;

// 设备名称缓冲区 (基础名称 + MAC 后3字节, 如 "Dell-PSU-Controller-A1B2C3")
static char g_device_name[64];

// BLE 发送互斥锁: 防止多个发送操作的分片在 BLE 栈中交错
// 主循环推送 get_data 和 on_ble_rx 响应 get_info 可能同时调用 ble_server_send
// 不加锁会导致两个 JSON 的分片交错, App 端无法解析
static SemaphoreHandle_t g_tx_mutex = NULL;


// ---------- 前向声明 ----------
static int ble_gap_event_cb(struct ble_gap_event *event, void *arg);
static int ble_gatt_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg);
static void ble_advertise(void);
static void ble_on_sync(void);

// ---------- GATT 服务定义 ----------

// NUS Service characteristics
static struct ble_gatt_chr_def g_nus_chrs[] = {
    {
        .uuid       = (ble_uuid_t*)&g_nus_tx_uuid,
        .access_cb  = ble_gatt_access_cb,
        .flags      = BLE_GATT_CHR_F_NOTIFY,
        .val_handle = &g_tx_val_handle,
    },
    {
        .uuid       = (ble_uuid_t*)&g_nus_rx_uuid,
        .access_cb  = ble_gatt_access_cb,
        .flags      = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
        .val_handle = nullptr,
    },
    {} // terminator
};

static const struct ble_gatt_svc_def g_svcs[] = {
    {
        .type     = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid     = (ble_uuid_t*)&g_nus_svc_uuid,
        .includes = nullptr,
        .characteristics = g_nus_chrs,
    },
    {} // terminator
};

// ---------- 实现 ----------

esp_err_t ble_server_init(void)
{
    int rc;

    // 重置状态
    g_conn_handle = 0;
    g_connected = false;
    g_synced = false;

    // 创建发送互斥锁
    if (g_tx_mutex == NULL) {
        g_tx_mutex = xSemaphoreCreateMutex();
        if (g_tx_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create TX mutex");
            return ESP_ERR_NO_MEM;
        }
    }

    // 擦除 NVS 中残留的蓝牙名称（之前改名功能遗留的乱码数据），恢复默认广播名
    {
        nvs_handle_t nvs_handle;
        esp_err_t err = nvs_open("ble", NVS_READWRITE, &nvs_handle);
        if (err == ESP_OK) {
            err = nvs_erase_key(nvs_handle, "device_name");
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Erased stale NVS device_name, will use default");
            }
            nvs_commit(nvs_handle);
            nvs_close(nvs_handle);
        }
    }

    // 构建设备名称: 基础名称 + "-" + MAC 后 3 字节
    {
        uint8_t mac[6];
        esp_err_t err = esp_read_mac(mac, ESP_MAC_BT);
        if (err == ESP_OK) {
            snprintf(g_device_name, sizeof(g_device_name),
                     "%s-%02X%02X%02X",
                     BLE_DEVICE_NAME, mac[3], mac[4], mac[5]);
        } else {
            // 读取 MAC 失败, 回退到无后缀名称
            snprintf(g_device_name, sizeof(g_device_name), "%s", BLE_DEVICE_NAME);
        }
        ESP_LOGI(TAG, "Device name: %s", g_device_name);
    }

    // 初始化标准 GAP 和 GATT 服务
    ble_svc_gap_init();
    ble_svc_gatt_init();

    // 注册 GATT 服务计数
    rc = ble_gatts_count_cfg(g_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_count_cfg failed: %d", rc);
        return ESP_FAIL;
    }

    // 添加 GATT 服务
    rc = ble_gatts_add_svcs(g_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "ble_gatts_add_svcs failed: %d", rc);
        return ESP_FAIL;
    }

    // 设置设备名称（基础名称 + MAC 后 3 字节后缀，便于区分同型号设备）
    rc = ble_svc_gap_device_name_set(g_device_name);
    if (rc != 0) {
        ESP_LOGW(TAG, "Device name set failed: %d", rc);
    }

    // 注册主机同步回调 (主机同步后自动开始广播)
    ble_hs_cfg.sync_cb = ble_on_sync;

    ESP_LOGI(TAG, "BLE server initialized, waiting for host sync...");
    return ESP_OK;
}

/** 主机同步回调: NimBLE 协议栈就绪后调用 */
static void ble_on_sync(void)
{
    ESP_LOGI(TAG, "NimBLE host synced, starting advertising...");
    g_synced = true;
    ble_advertise();
}

esp_err_t ble_server_send(const char* data)
{
    if (!g_connected || g_conn_handle == 0) {
        return ESP_ERR_INVALID_STATE;
    }

    // 获取发送互斥锁: 防止主循环推送和命令响应同时发送导致分片交错
    // 如果锁被占用, 等待最多 5 秒 (正常情况下不应等待这么久)
    if (g_tx_mutex != NULL) {
        if (xSemaphoreTake(g_tx_mutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
            ESP_LOGW(TAG, "TX mutex timeout (5s), dropping message");
            return ESP_ERR_TIMEOUT;
        }
    }

    size_t total_len = strlen(data);

    // 动态获取当前 MTU，计算分包大小
    // 典型 MTU: Android 默认 23 ~ 512，iOS 默认 23 ~ 185
    uint16_t mtu = ble_att_mtu(g_conn_handle);
    const size_t CHUNK_SIZE = (mtu > 4) ? (size_t)(mtu - 4) : 19; // 预留 3 字节协议头 + 1 字节 '\n'

    esp_err_t result = ESP_OK;

    if (total_len <= CHUNK_SIZE) {
        // 数据小, 直接发送 (末尾加 '\n')
        char buf[CHUNK_SIZE + 2];
        memcpy(buf, data, total_len);
        buf[total_len] = '\n';
        size_t buf_len = total_len + 1;

        struct os_mbuf *om = ble_hs_mbuf_from_flat(buf, buf_len);
        if (!om) {
            result = ESP_ERR_NO_MEM;
            goto cleanup;
        }

        int rc = ble_gattc_notify_custom(g_conn_handle, g_tx_val_handle, om);
        if (rc != 0) {
            ESP_LOGW(TAG, "Notify failed: %d", rc);
            result = ESP_FAIL;
            goto cleanup;
        }
        ESP_LOGD(TAG, "BLE TX (%zu bytes): %s", total_len, data);
    } else {
        // 大数据, 分包发送
        ESP_LOGD(TAG, "BLE TX: splitting %zu bytes into chunks", total_len);

        size_t offset = 0;
        int chunk_num = 0;
        while (offset < total_len) {
            size_t remaining = total_len - offset;
            bool is_last = (remaining <= CHUNK_SIZE);
            size_t chunk_size = is_last ? remaining : CHUNK_SIZE;

            char chunk[CHUNK_SIZE + 2];
            memcpy(chunk, data + offset, chunk_size);
            size_t chunk_len = chunk_size;

            if (is_last) {
                chunk[chunk_len] = '\n';
                chunk_len++;
            }

            struct os_mbuf *om = ble_hs_mbuf_from_flat(chunk, chunk_len);
            if (!om) {
                ESP_LOGW(TAG, "Chunk %d: alloc failed", chunk_num);
                result = ESP_ERR_NO_MEM;
                goto cleanup;
            }

            int rc = ble_gattc_notify_custom(g_conn_handle, g_tx_val_handle, om);
            if (rc != 0) {
                ESP_LOGW(TAG, "Chunk %d notify failed: %d", chunk_num, rc);
                result = ESP_FAIL;
                goto cleanup;
            }

            ESP_LOGV(TAG, "Chunk %d: %zu bytes (offset %zu/%zu)%s",
                     chunk_num, chunk_len, offset, total_len,
                     is_last ? " [LAST]" : "");
            offset += chunk_size;
            chunk_num++;

            vTaskDelay(pdMS_TO_TICKS(5));
        }

        ESP_LOGD(TAG, "BLE TX complete: %d chunks, %zu total bytes", chunk_num, total_len);
    }

cleanup:
    // 释放发送互斥锁
    if (g_tx_mutex != NULL) {
        xSemaphoreGive(g_tx_mutex);
    }
    return result;
}

bool ble_server_is_connected(void)
{
    return g_connected;
}

void ble_server_set_rx_callback(ble_rx_callback_t cb)
{
    g_rx_callback = cb;
}

const char* ble_server_get_device_name(void)
{
    return g_device_name;
}

// ---------- 静态函数 ----------

static void ble_advertise(void)
{
    int rc;

    // ========== 广播包 (31 bytes 限制) ==========
    // 只放 flags + 设备名称 (约 3 + 20 = 23 bytes, 足够)
    struct ble_hs_adv_fields advFields = {};
    memset(&advFields, 0, sizeof(advFields));

    // 设置广播标志: 支持 BLE 通用发现模式 + BR/EDR 不支持
    advFields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    // 添加设备名称到广播包
    advFields.name = (uint8_t*)g_device_name;
    advFields.name_len = strlen(g_device_name);
    advFields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&advFields);
    if (rc != 0) {
        ESP_LOGE(TAG, "Advertising fields set failed: %d", rc);
        return;
    }

    // ========== 扫描响应包 (31 bytes) ==========
    // 放 128-bit NUS 服务 UUID (17 bytes), 让 App 能通过服务 UUID 过滤
    struct ble_hs_adv_fields scanRspFields = {};
    memset(&scanRspFields, 0, sizeof(scanRspFields));

    // 添加 NUS 服务 UUID (128-bit) 到扫描响应
    scanRspFields.uuids128 = (ble_uuid128_t*)&g_nus_svc_uuid;
    scanRspFields.num_uuids128 = 1;
    scanRspFields.uuids128_is_complete = 1;

    rc = ble_gap_adv_rsp_set_fields(&scanRspFields);
    if (rc != 0) {
        ESP_LOGW(TAG, "Scan response fields set failed: %d", rc);
    }

    // 开始广播
    struct ble_gap_adv_params advParams = {};
    memset(&advParams, 0, sizeof(advParams));

    advParams.conn_mode      = BLE_GAP_CONN_MODE_UND;
    advParams.disc_mode      = BLE_GAP_DISC_MODE_GEN;
    advParams.itvl_min       = 32;  // 20ms
    advParams.itvl_max       = 160; // 100ms

    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, nullptr, BLE_HS_FOREVER,
                           &advParams, ble_gap_event_cb, nullptr);
    if (rc != 0) {
        ESP_LOGE(TAG, "Advertising start failed: %d", rc);
    }
}

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg)
{
    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        // 连接成功
        if (event->connect.status == 0) {
            g_conn_handle = event->connect.conn_handle;
            g_connected   = true;
            ESP_LOGI(TAG, "Client connected (handle=%d)", g_conn_handle);
            // 请求 MTU 升级到 512 字节, 减少分包
            // 注意: MTU 交换是异步的, 主循环中首次推送数据时可能 MTU 还没完成
            // 但 NimBLE 会自动处理, 通知会在 MTU 交换完成后才发送
            int rc = ble_gattc_exchange_mtu(g_conn_handle, nullptr, nullptr);
            if (rc != 0) {
                ESP_LOGW(TAG, "MTU exchange request failed: %d", rc);
            } else {
                ESP_LOGI(TAG, "MTU exchange requested (target: 512)");
            }
        } else {
            ESP_LOGW(TAG, "Connection failed, restart advertising");
            ble_advertise();
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Client disconnected (reason=%d)", event->disconnect.reason);
        g_conn_handle = 0;
        g_connected   = false;
        // 重新开始广播
        ble_advertise();
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        // 广播超时, 重新开始
        ESP_LOGI(TAG, "Advertising timeout, restarting");
        ble_advertise();
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "Client subscribed to notifications");
        return 0;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "MTU updated: %d", event->mtu.value);
        return 0;

    default:
        return 0;
    }
}

static int ble_gatt_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    ESP_LOGD(TAG, "GATT access: op=%d conn_handle=%d attr_handle=%d",
             ctxt->op, conn_handle, attr_handle);

    // 处理 RX 特征值的写入 (App → ESP32)
    // NimBLE 中所有写操作 (带响应和不带响应) 都通过 BLE_GATT_ACCESS_OP_WRITE_CHR 路由
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        if (g_rx_callback && ctxt->om) {
            // 从 mbuf 中读取数据
            uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
            ESP_LOGD(TAG, "RX: om_len=%d", om_len);
            if (om_len > 0 && om_len < 512) {
                char buf[512] = {};
                uint16_t copy_len = 0;
                int rc = ble_hs_mbuf_to_flat(ctxt->om, buf, sizeof(buf) - 1, &copy_len);
                if (rc == 0 && copy_len > 0) {
                    buf[copy_len] = '\0';
                    ESP_LOGI(TAG, "BLE RX (%d bytes): %s", copy_len, buf);
                    g_rx_callback(buf, copy_len);
                } else {
                    ESP_LOGW(TAG, "RX: ble_hs_mbuf_to_flat rc=%d copy_len=%d", rc, copy_len);
                }
            } else {
                ESP_LOGW(TAG, "RX: invalid data length %d", om_len);
            }
        } else {
            ESP_LOGW(TAG, "RX: g_rx_callback=%p ctxt->om=%p", g_rx_callback, ctxt->om);
        }
        return 0;
    }

    // 其他操作 (读、描述符写等) 返回错误
    return BLE_ATT_ERR_UNLIKELY;
}

// ---------- NimBLE 主机任务 ----------

/** NimBLE 主机任务: 运行 BLE 协议栈事件循环 */
extern "C" void nimble_host_task(void *param)
{
    ESP_LOGI(TAG, "NimBLE host task started");
    nimble_port_run();
    ESP_LOGI(TAG, "NimBLE host task exited");
}
