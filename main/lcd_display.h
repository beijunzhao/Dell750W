/**
 * lcd_display.h - ST7789 TFT 显示屏驱动封装
 * 硬件平台: ESP32-C3
 * 驱动 IC: ST7789 (2.01英寸, 240x296 RGB565)
 * SPI 接口
 */
#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "esp_err.h"
#include "driver/gpio.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 ST7789 显示屏
 *
 * 根据 pin_map.h 中的引脚定义配置 SPI 总线并初始化 ST7789 面板。
 * 自动补偿 2.01 英寸屏幕的 GRAM 偏移 (Y 轴偏移 24 像素)。
 *
 * @return esp_err_t ESP_OK 成功，否则失败
 */
esp_err_t lcd_display_init(void);

/**
 * @brief 反初始化显示屏，释放资源
 *
 * @return esp_err_t ESP_OK 成功
 */
esp_err_t lcd_display_deinit(void);

/**
 * @brief 获取显示屏宽度
 *
 * @return uint16_t 宽度（像素）
 */
uint16_t lcd_display_get_width(void);

/**
 * @brief 获取显示屏高度
 *
 * @return uint16_t 高度（像素）
 */
uint16_t lcd_display_get_height(void);

/**
 * @brief 局部刷新：发送指定矩形区域的帧缓冲区到显示屏
 *
 * 支持任意矩形区域的局部刷新，避免全屏 DMA 传输导致的内存耗尽。
 * 坐标系为左上角原点 (0,0)，右下角 (width-1, height-1)。
 *
 * @param x1            矩形区域左上角 X 坐标（包含）
 * @param y1            矩形区域左上角 Y 坐标（包含）
 * @param x2            矩形区域右下角 X 坐标（包含）
 * @param y2            矩形区域右下角 Y 坐标（包含）
 * @param frame_buffer  RGB565 格式的帧缓冲区数据指针。
 *                      缓冲区大小必须 >= (x2-x1+1) * (y2-y1+1) * 2 字节。
 *                      数据按行排列，每行从左到右连续存储。
 * @return esp_err_t    ESP_OK 成功，否则失败
 */
esp_err_t lcd_display_send_frame(uint16_t x1, uint16_t y1,
                                 uint16_t x2, uint16_t y2,
                                 const void *frame_buffer);

/**
 * @brief 检查显示屏是否已初始化
 *
 * @return true 已初始化
 * @return false 未初始化
 */
bool lcd_display_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif // LCD_DISPLAY_H
