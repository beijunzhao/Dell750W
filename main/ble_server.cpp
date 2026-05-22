/**
 * ble_server.cpp - BLE NimBLE GATT 服务器实现
 *
 * Nordic UART Service (NUS) profile
 * 参考: ESP-IDF NimBLE examples
 */
#include "ble_server.h"
#include "esp_log.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include <cstring>

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

    // 设置设备名称
    rc = ble_svc_gap_device_name_set(BLE_DEVICE_NAME);
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

    size_t total_len = strlen(data);

    // MTU=23 时有效载荷仅 20 字节
    // 如果数据超过 20 字节, NimBLE 的 ble_gattc_notify_custom 会自动分片
    // 但某些 Android 设备可能无法正确处理自动分片
    // 因此我们手动分包: 每包不超过 19 字节, 最后一包末尾加 '\n'
    // App 端的粘包处理逻辑 (按 '\n' 分割) 会重组完整 JSON
    const size_t CHUNK_SIZE = 19; // 留 1 字节给 '\n'

    if (total_len <= CHUNK_SIZE) {
        // 数据小, 直接发送 (末尾加 '\n')
        char buf[CHUNK_SIZE + 2];
        memcpy(buf, data, total_len);
        buf[total_len] = '\n';
        size_t buf_len = total_len + 1;

        struct os_mbuf *om = ble_hs_mbuf_from_flat(buf, buf_len);
        if (!om) return ESP_ERR_NO_MEM;

        int rc = ble_gattc_notify_custom(g_conn_handle, g_tx_val_handle, om);
        if (rc != 0) {
            ESP_LOGW(TAG, "Notify failed: %d", rc);
            return ESP_FAIL;
        }
        ESP_LOGI(TAG, "BLE TX (%zu bytes): %s", total_len, data);
        return ESP_OK;
    }

    // 大数据, 分包发送
    ESP_LOGI(TAG, "BLE TX: splitting %zu bytes into chunks", total_len);

    size_t offset = 0;
    int chunk_num = 0;
    while (offset < total_len) {
        // 计算本包大小
        size_t remaining = total_len - offset;
        bool is_last = (remaining <= CHUNK_SIZE);
        size_t chunk_size = is_last ? remaining : CHUNK_SIZE;

        // 构建本包数据
        // - 中间分片: 纯数据, 不加 '\n' (App 端会累积到 _receiveBuffer)
        // - 最后一个分片: 数据 + '\n' (App 端按 '\n' 分割, 得到完整 JSON)
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
            return ESP_ERR_NO_MEM;
        }

        int rc = ble_gattc_notify_custom(g_conn_handle, g_tx_val_handle, om);
        if (rc != 0) {
            ESP_LOGW(TAG, "Chunk %d notify failed: %d", chunk_num, rc);
            return ESP_FAIL;
        }

        ESP_LOGD(TAG, "Chunk %d: %zu bytes (offset %zu/%zu)%s",
                 chunk_num, chunk_len, offset, total_len,
                 is_last ? " [LAST]" : "");
        offset += chunk_size;
        chunk_num++;

        // 短延时, 给 BLE 栈时间处理
        vTaskDelay(pdMS_TO_TICKS(15));
    }

    ESP_LOGI(TAG, "BLE TX complete: %d chunks, %zu total bytes", chunk_num, total_len);
    return ESP_OK;
}

bool ble_server_is_connected(void)
{
    return g_connected;
}

void ble_server_set_rx_callback(ble_rx_callback_t cb)
{
    g_rx_callback = cb;
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
    advFields.name = (uint8_t*)BLE_DEVICE_NAME;
    advFields.name_len = strlen(BLE_DEVICE_NAME);
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
