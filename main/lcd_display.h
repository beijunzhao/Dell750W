/**
 * lcd_display.h - ST7789P3 2.01" 240x296 TFT 驱动
 */
#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_io.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 屏幕尺寸 ---- */
#define LCD_WIDTH   240
#define LCD_HEIGHT  296

/* ---- 常用 RGB565 颜色 ---- */
#define LCD_COLOR_BLACK   0x0000
#define LCD_COLOR_WHITE   0xFFFF
#define LCD_COLOR_RED     0xF800
#define LCD_COLOR_GREEN   0x07E0
#define LCD_COLOR_BLUE    0x001F

/* ---- API ---- */

/** 初始化 SPI 总线 + ST7789P3 面板 + 清屏 */
esp_err_t lcd_display_init(void);

/** 释放资源 */
esp_err_t lcd_display_deinit(void);

/** 宽度 */
uint16_t lcd_display_get_width(void);

/** 高度 */
uint16_t lcd_display_get_height(void);

/**
 * @brief 矩形区域刷新 (局部传输, 避免 DMA OOM)
 * @param x1  左上角 X (包含)
 * @param y1  左上角 Y (包含)
 * @param x2  右下角 X (包含)
 * @param y2  右下角 Y (包含)
 * @param frame_buffer  RGB565 像素数据, 大小 >= (x2-x1+1)*(y2-y1+1)*2
 */
esp_err_t lcd_display_send_frame(uint16_t x1, uint16_t y1,
                                 uint16_t x2, uint16_t y2,
                                 const void *frame_buffer);

/** 是否已初始化 */
bool lcd_display_is_initialized(void);

/** 全屏纯色填充 */
esp_err_t lcd_display_fill_screen(uint16_t color);

/** 获取 Panel IO 句柄 (供 LVGL) */
esp_lcd_panel_io_handle_t lcd_display_get_io(void);

/** 获取 Panel 句柄 (供 LVGL) */
esp_lcd_panel_handle_t lcd_display_get_panel(void);

#ifdef __cplusplus
}
#endif

#endif
