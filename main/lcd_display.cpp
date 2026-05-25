/**
 * lcd_display.cpp - ST7789P3 2.01" 240x296 TFT 驱动 (ESP-IDF)
 *
 * ===== 核心设计 =====
 * 1. DMA 安全: max_transfer_sz 限制为 40 行 (~19KB), 避免 ESP32-C3 OOM
 * 2. GRAM 偏移: set_gap(0,0) 可见区起于 GRAM 第 0 行
 * 3. 局部刷新: 所有写屏操作必须传入坐标，杜绝全屏 DMA 传输
 * 4. ST7789P3 专用初始化: 补充厂商 Gamma/VCOM/FrameRate 寄存器
 *
 * 引脚见 pin_map.h
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

static const char *TAG = "LCD";

#define BITS_PER_PX        16
#define GRAM_OFFSET_Y      0       // ST7789P3 此面板可见区起于 GRAM 第 0 行
#define MAX_LINES_PER_XFER 40      // 每 chunk 最大行数
#define SPI_CLK_HZ         (20*1000*1000)  // 降到 20MHz 提高时序裕量

// ---- 字节序: 0=BIG (标准), 1=LITTLE ----
#define LCD_ENDIAN_LITTLE  1

// ---- 颜色翻转: 0=不翻, 1=翻转 ----
#define LCD_INVERT_COLOR   0

// ---- SPI 模式: 0 或 3 ----
#define LCD_SPI_MODE       0

#define SPI_HOST           SPI2_HOST

// ---- 传输缓冲上限 ----
#define XFER_SZ   (LCD_WIDTH * MAX_LINES_PER_XFER * 2 + 32)

/* ========== 静态变量 ========== */

static esp_lcd_panel_handle_t    s_panel = NULL;
static esp_lcd_panel_io_handle_t s_io    = NULL;
static SemaphoreHandle_t         s_sem   = NULL;
static bool                      s_ok    = false;

/* ========== ISR 回调 ========== */

static bool IRAM_ATTR on_done(esp_lcd_panel_io_handle_t h,
                              esp_lcd_panel_io_event_data_t *e,
                              void *ctx)
{
    SemaphoreHandle_t s = (SemaphoreHandle_t)ctx;
    BaseType_t w = pdFALSE;
    if (s) xSemaphoreGiveFromISR(s, &w);
    return (w == pdTRUE);
}

/* ========== 原始寄存器写入 ========== */

static void wr_cmd(uint8_t cmd) {
    esp_lcd_panel_io_tx_param(s_io, cmd, NULL, 0);
}
static void wr_data(uint8_t d) {
    esp_lcd_panel_io_tx_param(s_io, 0, &d, 1);  // 0=延续前一个cmd
}

static void st7789p3_init_seq(void)
{
    // 参考 ST7789P3 厂商初始化序列
    // 这些寄存器 ESP-IDF 默认 init 可能未设置或值不同

    // ---- VCOMS (BBh) ----
    wr_cmd(0xBB); wr_data(0x19);

    // ---- Power Control 1 (C2h) ----
    wr_cmd(0xC2); wr_data(0x01); wr_data(0xFF);

    // ---- VRH Set (C3h) ----
    wr_cmd(0xC3); wr_data(0x12);

    // ---- VDV Set (C4h) ----
    wr_cmd(0xC4); wr_data(0x20);

    // ---- Frame Rate Control (C6h) ----
    wr_cmd(0xC6); wr_data(0x0F);

    // ---- Power Control 2 (D0h) ----
    wr_cmd(0xD0); wr_data(0xA4); wr_data(0xA1);

    // ---- Positive Gamma (E0h) ----
    wr_cmd(0xE0);
    {
        static const uint8_t g[] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
        esp_lcd_panel_io_tx_param(s_io, 0, g, sizeof(g));
    }

    // ---- Negative Gamma (E1h) ----
    wr_cmd(0xE1);
    {
        static const uint8_t g[] = {0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
        esp_lcd_panel_io_tx_param(s_io, 0, g, sizeof(g));
    }

    // ---- 显式设置 CASET 列地址 (2Ah): 0~239 ----
    {
        const uint8_t v[] = {0x00, 0x00, 0x00, 0xEF}; // start=0, end=239
        wr_cmd(0x2A);
        esp_lcd_panel_io_tx_param(s_io, 0, v, sizeof(v));
    }

    // ---- 显式设置 RASET 行地址 (2Bh): 0~319 ----
    {
        const uint8_t v[] = {0x00, 0x00, 0x01, 0x3F}; // start=0, end=319
        wr_cmd(0x2B);
        esp_lcd_panel_io_tx_param(s_io, 0, v, sizeof(v));
    }

    // ---- MADCTL (36h) ----
    {
        const uint8_t v = 0x00; // MY=0 MX=0 MV=0 RGB=0
        wr_cmd(0x36); wr_data(v);
    }

    // ---- Normal Display Mode (13h) ----
    wr_cmd(0x13);

    ESP_LOGI(TAG, "ST7789P3 init sequence done");
}

/* ========== 公开接口 ========== */

esp_err_t lcd_display_init(void)
{
    if (s_ok) { ESP_LOGW(TAG, "Already init"); return ESP_OK; }

    ESP_LOGI(TAG, "===== ST7789P3 240x296 init =====");
    ESP_LOGI(TAG, "Pins: SCLK=%d MOSI=%d DC=%d RST=%d CS=%d",
             TFT_SCLK, TFT_MOSI, TFT_DC, TFT_RST, TFT_CS);
    ESP_LOGI(TAG, "SPI: clk=%dMHz mode=%d endian=%s invert=%d",
             SPI_CLK_HZ/1000000, LCD_SPI_MODE,
             LCD_ENDIAN_LITTLE ? "LITTLE" : "BIG",
             LCD_INVERT_COLOR);

    esp_err_t r;

    // ---- 1. SPI 总线 ----
    spi_bus_config_t b = {
        .mosi_io_num     = TFT_MOSI,
        .miso_io_num     = -1,
        .sclk_io_num     = TFT_SCLK,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = (int)XFER_SZ,
    };
    r = spi_bus_initialize(SPI_HOST, &b, SPI_DMA_CH_AUTO);
    if (r != ESP_OK) { ESP_LOGE(TAG, "SPI bus fail: %s", esp_err_to_name(r)); return r; }
    ESP_LOGD(TAG, "SPI bus OK, xfer_sz=%d", (int)XFER_SZ);

    // ---- 2. 信号量 ----
    s_sem = xSemaphoreCreateBinary();
    if (!s_sem) { spi_bus_free(SPI_HOST); return ESP_ERR_NO_MEM; }
    xSemaphoreGive(s_sem);

    // ---- 3. Panel IO (SPI) ----
    esp_lcd_panel_io_spi_config_t io_c = {
        .cs_gpio_num          = TFT_CS,
        .dc_gpio_num          = TFT_DC,
        .spi_mode             = LCD_SPI_MODE,
        .pclk_hz              = SPI_CLK_HZ,
        .trans_queue_depth    = 10,
        .on_color_trans_done  = on_done,
        .user_ctx             = s_sem,
        .lcd_cmd_bits         = 8,
        .lcd_param_bits       = 8,
        .cs_ena_pretrans      = 8,   // CS 提前 8 个 SPI 周期使能
        .cs_ena_posttrans     = 8,   // CS 延后 8 个 SPI 周期释放
    };
    r = esp_lcd_new_panel_io_spi(SPI_HOST, &io_c, &s_io);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "IO fail: %s", esp_err_to_name(r));
        vSemaphoreDelete(s_sem); s_sem = NULL;
        spi_bus_free(SPI_HOST); return r;
    }
    ESP_LOGD(TAG, "Panel IO OK");

    // ---- 4. 创建 ST7789 面板 ----
    esp_lcd_panel_dev_config_t p_c = {
        .rgb_ele_order    = LCD_RGB_ELEMENT_ORDER_RGB,
#if LCD_ENDIAN_LITTLE
        .data_endian      = LCD_RGB_DATA_ENDIAN_LITTLE,
#else
        .data_endian      = LCD_RGB_DATA_ENDIAN_BIG,
#endif
        .bits_per_pixel   = BITS_PER_PX,
        .reset_gpio_num   = TFT_RST,
        .vendor_config    = NULL,
        .flags            = {
            .reset_active_high = false,
        },
    };
    r = esp_lcd_new_panel_st7789(s_io, &p_c, &s_panel);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "Panel fail: %s", esp_err_to_name(r));
        esp_lcd_panel_io_del(s_io); s_io = NULL;
        vSemaphoreDelete(s_sem); s_sem = NULL;
        spi_bus_free(SPI_HOST); return r;
    }

    // ---- 5. 复位 + ESP-IDF 标准初始化 ----
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));

    // ---- 6. ST7789P3 专用寄存器序列 ----
    st7789p3_init_seq();

    // ---- 7. GRAM 偏移 ----
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(s_panel, 0, GRAM_OFFSET_Y));
    ESP_LOGD(TAG, "Gap: x=0 y=%d", GRAM_OFFSET_Y);

    // ---- 8. 颜色翻转 ----
#if LCD_INVERT_COLOR
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_panel, true));
    ESP_LOGD(TAG, "Invert: ON");
#else
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_panel, false));
    ESP_LOGD(TAG, "Invert: OFF");
#endif

    // ---- 9. 开显示 ----
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel, true));

    // ---- 10. 清屏 ----
    lcd_display_fill_screen(LCD_COLOR_BLACK);

    s_ok = true;
    ESP_LOGI(TAG, "===== Init OK (%dx%d) =====", LCD_WIDTH, LCD_HEIGHT);
    return ESP_OK;
}

esp_err_t lcd_display_deinit(void)
{
    if (!s_ok) return ESP_OK;
    if (s_panel) { esp_lcd_panel_disp_on_off(s_panel, false); esp_lcd_panel_del(s_panel); s_panel = NULL; }
    if (s_io)    { esp_lcd_panel_io_del(s_io); s_io = NULL; }
    if (s_sem)   { vSemaphoreDelete(s_sem); s_sem = NULL; }
    spi_bus_free(SPI_HOST);
    s_ok = false;
    ESP_LOGI(TAG, "Deinit");
    return ESP_OK;
}

uint16_t lcd_display_get_width(void)  { return LCD_WIDTH; }
uint16_t lcd_display_get_height(void) { return LCD_HEIGHT; }

esp_err_t lcd_display_send_frame(uint16_t x1, uint16_t y1,
                                 uint16_t x2, uint16_t y2,
                                 const void *buf)
{
    if (!s_ok || !s_panel) return ESP_ERR_INVALID_STATE;
    if (x1 > x2 || y1 > y2 || x2 >= LCD_WIDTH || y2 >= LCD_HEIGHT) {
        ESP_LOGE(TAG, "Bad rect (%d,%d)-(%d,%d)", x1,y1,x2,y2);
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(s_sem, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGW(TAG, "Sem timeout");
        return ESP_ERR_TIMEOUT;
    }

    // draw_bitmap: end 坐标是 exclusive → +1
    esp_err_t r = esp_lcd_panel_draw_bitmap(s_panel, x1, y1, x2+1, y2+1, buf);
    if (r != ESP_OK) {
        ESP_LOGE(TAG, "draw_bitmap fail: %s", esp_err_to_name(r));
        xSemaphoreGive(s_sem);
    }
    return r;
}

bool lcd_display_is_initialized(void) { return s_ok; }

esp_lcd_panel_io_handle_t lcd_display_get_io(void)  { return s_io; }
esp_lcd_panel_handle_t    lcd_display_get_panel(void) { return s_panel; }

/* ========== 全屏纯色填充 ========== */

esp_err_t lcd_display_fill_screen(uint16_t color)
{
    if (!s_panel) return ESP_ERR_INVALID_STATE;

    const size_t n = (size_t)LCD_WIDTH * MAX_LINES_PER_XFER;
    const size_t sz = n * 2;
    uint16_t *buf = (uint16_t *)heap_caps_malloc(sz, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    if (!buf) { ESP_LOGE(TAG, "malloc fail"); return ESP_ERR_NO_MEM; }
    for (size_t i = 0; i < n; i++) buf[i] = color;

    esp_err_t r = ESP_OK;
    uint16_t y = 0;
    while (y < LCD_HEIGHT) {
        uint16_t h = MAX_LINES_PER_XFER;
        if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;

        if (xSemaphoreTake(s_sem, pdMS_TO_TICKS(2000)) != pdTRUE) {
            ESP_LOGE(TAG, "fill sem timeout @y=%d", y);
            r = ESP_ERR_TIMEOUT; break;
        }
        r = esp_lcd_panel_draw_bitmap(s_panel, 0, y, LCD_WIDTH, y+h, buf);
        if (r != ESP_OK) {
            ESP_LOGE(TAG, "fill draw @y=%d: %s", y, esp_err_to_name(r));
            xSemaphoreGive(s_sem); break;
        }
        y += h;
    }
    if (r == ESP_OK) {
        if (xSemaphoreTake(s_sem, pdMS_TO_TICKS(2000)) != pdTRUE)
            r = ESP_ERR_TIMEOUT;
        xSemaphoreGive(s_sem);
    }
    heap_caps_free(buf);
    return r;
}
