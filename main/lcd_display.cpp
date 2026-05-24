/**
 * lcd_display.cpp - ST7789 TFT 显示屏驱动封装实现
 * 硬件平台: ESP32-C3
 * 驱动 IC: ST7789 (2.01英寸, 240x296 RGB565)
 * SPI 接口
 *
 * 引脚定义 (来自 pin_map.h):
 *   TFT_SCLK = GPIO_NUM_6   (SPI 时钟)
 *   TFT_MOSI = GPIO_NUM_7   (SPI 数据)
 *   TFT_RST  = GPIO_NUM_8   (硬件复位)
 *   TFT_DC   = GPIO_NUM_9   (数据/命令选择)
 *   TFT_CS   = GPIO_NUM_10  (片选)
 *
 * 关键设计决策:
 *   1. max_transfer_sz 限制为 40 行数据，避免 ESP32-C3 DMA 内存耗尽 (OOM)
 *   2. 显式设置 GRAM 偏移 (gap_y=24) 补偿 2.01 英寸屏幕的物理裁切
 *   3. 竖屏 240x296，不交换 XY，不镜像
 *   4. 支持任意矩形区域局部刷新，避免全屏 DMA 传输
 */
#include "lcd_display.h"
#include "pin_map.h"

#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_io_spi.h"
#include "driver/spi_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "LCD_Display";

// 显示屏参数 (2.01英寸 ST7789, 240x296)
#define LCD_WIDTH           240
#define LCD_HEIGHT          296
#define LCD_BITS_PER_PIXEL  16   // RGB565
#define LCD_GRAM_OFFSET_Y   24   // ST7789 原生 320 行，物理裁切为 296 行，Y 偏移 24

// SPI 局部传输：每次最多 40 行，避免 DMA OOM
// 40 行 * 240 像素 * 2 字节 = 19200 字节，ESP32-C3 的 DMA 可轻松容纳
#define LCD_MAX_LINES_PER_XFER  40
#define LCD_XFER_SZ             (LCD_WIDTH * LCD_MAX_LINES_PER_XFER * 2 + 8)

// SPI 主机
#define LCD_SPI_HOST SPI2_HOST

// 静态变量
static esp_lcd_panel_handle_t s_lcd_panel = NULL;
static esp_lcd_panel_io_handle_t s_lcd_io = NULL;
static SemaphoreHandle_t s_refresh_sem = NULL;
static bool s_initialized = false;

// 刷新完成回调 (ISR 上下文)
static bool IRAM_ATTR lcd_refresh_callback(esp_lcd_panel_io_handle_t panel,
                                            esp_lcd_panel_io_event_data_t *edata,
                                            void *user_ctx)
{
    SemaphoreHandle_t sem = (SemaphoreHandle_t)user_ctx;
    if (sem) {
        BaseType_t need_yield = pdFALSE;
        xSemaphoreGiveFromISR(sem, &need_yield);
        return (need_yield == pdTRUE);
    }
    return false;
}

esp_err_t lcd_display_init(void)
{
    if (s_initialized) {
        ESP_LOGW(TAG, "LCD already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing ST7789 LCD panel (2.01\", 240x296)...");
    ESP_LOGI(TAG, "  SPI: SCLK=GPIO%d, MOSI=GPIO%d, CS=GPIO%d, DC=GPIO%d, RST=GPIO%d",
             TFT_SCLK, TFT_MOSI, TFT_CS, TFT_DC, TFT_RST);
    ESP_LOGI(TAG, "  GRAM offset: Y=%d, max_xfer_lines=%d, xfer_sz=%d bytes",
             LCD_GRAM_OFFSET_Y, LCD_MAX_LINES_PER_XFER, LCD_XFER_SZ);

    // ========== 1. 配置 SPI 总线 ==========
    // max_transfer_sz 设为局部传输大小，避免 DMA OOM
    spi_bus_config_t buscfg = {
        .mosi_io_num     = TFT_MOSI,
        .miso_io_num     = -1,
        .sclk_io_num     = TFT_SCLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = LCD_XFER_SZ,
    };

    esp_err_t ret = spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGD(TAG, "SPI bus initialized");

    // ========== 2. 创建信号量 (用于帧刷新同步) ==========
    s_refresh_sem = xSemaphoreCreateBinary();
    if (!s_refresh_sem) {
        ESP_LOGE(TAG, "Failed to create refresh semaphore");
        spi_bus_free(LCD_SPI_HOST);
        return ESP_ERR_NO_MEM;
    }
    xSemaphoreGive(s_refresh_sem);

    // ========== 3. 配置 SPI IO 接口 ==========
    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num        = TFT_CS,
        .dc_gpio_num        = TFT_DC,
        .spi_mode           = 0,
        .pclk_hz            = 40 * 1000 * 1000,  // 40MHz SPI 时钟
        .trans_queue_depth  = 10,
        .on_color_trans_done = lcd_refresh_callback,
        .user_ctx           = s_refresh_sem,
        .lcd_cmd_bits       = 8,
        .lcd_param_bits     = 8,
    };

    ret = esp_lcd_new_panel_io_spi(LCD_SPI_HOST, &io_config, &s_lcd_io);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel IO SPI init failed: %s", esp_err_to_name(ret));
        vSemaphoreDelete(s_refresh_sem);
        spi_bus_free(LCD_SPI_HOST);
        return ret;
    }
    ESP_LOGD(TAG, "Panel IO SPI initialized");

    // ========== 4. 创建 ST7789 面板 ==========
    esp_lcd_panel_dev_config_t panel_config = {
        .data_endian        = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel     = LCD_BITS_PER_PIXEL,
        .reset_gpio_num     = TFT_RST,
        .flags = {
            .reset_active_high = false,
        },
    };

    ret = esp_lcd_new_panel_st7789(s_lcd_io, &panel_config, &s_lcd_panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ST7789 panel: %s", esp_err_to_name(ret));
        esp_lcd_panel_io_del(s_lcd_io);
        vSemaphoreDelete(s_refresh_sem);
        spi_bus_free(LCD_SPI_HOST);
        return ret;
    }
    ESP_LOGD(TAG, "ST7789 panel created");

    // ========== 5. 初始化面板 ==========
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_lcd_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_lcd_panel));

    // ========== 6. 设置 GRAM 偏移补偿物理裁切 ==========
    // ST7789 原生 GRAM 为 240x320，2.01 英寸屏幕物理裁切为 240x296
    // 设置 Y 轴偏移 24 像素，使 CASET/RASET 命令的坐标正确映射到可见区域
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(s_lcd_panel, 0, LCD_GRAM_OFFSET_Y));
    ESP_LOGD(TAG, "GRAM gap set: x=0, y=%d", LCD_GRAM_OFFSET_Y);

    // ========== 7. 竖屏配置 ==========
    // 2.01 英寸屏幕为竖屏 240x296，不交换 XY
    // 颜色翻转：某些 ST7789 模组需要翻转颜色才能正确显示 RGB565
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_lcd_panel, true));

    // 打开显示
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_lcd_panel, true));

    s_initialized = true;
    ESP_LOGI(TAG, "ST7789 LCD initialized: %dx%d @ %dbpp, GRAM offset Y=%d",
             LCD_WIDTH, LCD_HEIGHT, LCD_BITS_PER_PIXEL, LCD_GRAM_OFFSET_Y);

    return ESP_OK;
}

esp_err_t lcd_display_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    if (s_lcd_panel) {
        esp_lcd_panel_disp_on_off(s_lcd_panel, false);
        esp_lcd_panel_del(s_lcd_panel);
        s_lcd_panel = NULL;
    }

    if (s_lcd_io) {
        esp_lcd_panel_io_del(s_lcd_io);
        s_lcd_io = NULL;
    }

    if (s_refresh_sem) {
        vSemaphoreDelete(s_refresh_sem);
        s_refresh_sem = NULL;
    }

    spi_bus_free(LCD_SPI_HOST);

    s_initialized = false;
    ESP_LOGI(TAG, "LCD deinitialized");

    return ESP_OK;
}

uint16_t lcd_display_get_width(void)
{
    return LCD_WIDTH;
}

uint16_t lcd_display_get_height(void)
{
    return LCD_HEIGHT;
}

esp_err_t lcd_display_send_frame(uint16_t x1, uint16_t y1,
                                 uint16_t x2, uint16_t y2,
                                 const void *frame_buffer)
{
    if (!s_initialized || !s_lcd_panel) {
        return ESP_ERR_INVALID_STATE;
    }

    // 边界检查
    if (x1 > x2 || y1 > y2 || x2 >= LCD_WIDTH || y2 >= LCD_HEIGHT) {
        ESP_LOGE(TAG, "Invalid rect: (%d,%d)-(%d,%d), max (%d,%d)",
                 x1, y1, x2, y2, LCD_WIDTH - 1, LCD_HEIGHT - 1);
        return ESP_ERR_INVALID_ARG;
    }

    // 等待上一帧传输完成
    if (xSemaphoreTake(s_refresh_sem, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGW(TAG, "Timeout waiting for refresh semaphore");
        return ESP_ERR_TIMEOUT;
    }

    // esp_lcd_panel_draw_bitmap 的坐标规则：
    //   传入的 x2, y2 是 排他性 (exclusive) 边界
    //   即实际绘制的区域为 [x1, x2) x [y1, y2)
    //   因此调用方传入包含性 (inclusive) 坐标时，需要 +1
    esp_err_t ret = esp_lcd_panel_draw_bitmap(s_lcd_panel,
                                               x1, y1,
                                               x2 + 1, y2 + 1,
                                               frame_buffer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to draw bitmap [%d,%d - %d,%d]: %s",
                 x1, y1, x2, y2, esp_err_to_name(ret));
        // 释放信号量以便下次重试
        xSemaphoreGive(s_refresh_sem);
    }

    return ret;
}

bool lcd_display_is_initialized(void)
{
    return s_initialized;
}
