/**
 * lvgl_setup.h - LVGL 显示初始化 (基于 esp_lvgl_port)
 *
 * 依赖:
 *   espressif/esp_lvgl_port ^2.8.0~1
 *   lvgl/lvgl              ^9
 */
#ifndef LVGL_SETUP_H
#define LVGL_SETUP_H

#include "esp_err.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 LVGL 并绑定 ST7789 显示面板
 *
 * @param io_handle   已初始化的 SPI Panel IO 句柄
 * @param panel_handle 已初始化的 ST7789 Panel 句柄
 * @return ESP_OK 成功
 */
esp_err_t lvgl_setup_init(esp_lcd_panel_io_handle_t io_handle,
                          esp_lcd_panel_handle_t panel_handle);

/**
 * @brief 检查 LVGL 是否已就绪
 * @return true 已就绪
 */
bool lvgl_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_SETUP_H */
