/**
 * lvgl_setup.cpp - LVGL 显示初始化 (基于 esp_lvgl_port)
 *
 * 硬件平台: ESP32-C3
 * 屏幕: 2.01英寸 ST7789P3, 240x296 (竖屏) → LVGL 软件旋转 270° 实现横屏
 *
 * 横屏方案: 硬件保持 MADCTL=0x00 (竖屏, MV=0),
 * LVGL lv_display_set_rotation(270°) 在软件层旋转 UI,
 * 避免 MV=1 导致的 GRAM 填充方向与 LVGL 扫描线不匹配问题.
 */
#include "lvgl_setup.h"

#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LVGL";
static bool s_ready = false;

/* ========== LVGL + ST7789 初始化 ========== */

esp_err_t lvgl_setup_init(esp_lcd_panel_io_handle_t io,
                          esp_lcd_panel_handle_t panel)
{
    if (!io || !panel) {
        ESP_LOGE(TAG, "Invalid handle: io=%p panel=%p", (void *)io, (void *)panel);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "========== LVGL Init Start ==========");

    // 1. 初始化 LVGL 核心
    lv_init();

    // 2. 初始化 LVGL Port (创建渲染任务 + 定时器)
    lvgl_port_cfg_t port_cfg = {
        .task_priority    = 4,
        .task_stack       = 4096,
        .task_affinity    = -1,
        .task_max_sleep_ms = 500,
        .timer_period_ms  = 5,
    };
    esp_err_t ret = lvgl_port_init(&port_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "lvgl_port_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "LVGL port initialized");

    // 3. 添加 ST7789 显示设备 (物理竖屏 240x296, LVGL 软件旋转到横屏)
    // buffer_size: 40 行 × 240 像素 = 9600 像素 ≈ 19KB (单缓冲)
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle    = io,
        .panel_handle = panel,
        .buffer_size  = 240 * 40,
        .double_buffer = false,
        .hres         = 240,
        .vres         = 296,
        .monochrome   = false,
        .rotation = {
            .swap_xy  = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_spiram = false,
            .swap_bytes  = false,
        },
    };

    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);
    if (!disp) {
        ESP_LOGE(TAG, "lvgl_port_add_disp failed");
        lvgl_port_deinit();
        return ESP_FAIL;
    }

    // LVGL 软件旋转 270° → 逻辑坐标 296×240 横屏
    lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
    ESP_LOGI(TAG, "Display added: physical %dx%d, logical landscape %dx%d",
             disp_cfg.hres, disp_cfg.vres,
             lv_display_get_horizontal_resolution(disp),
             lv_display_get_vertical_resolution(disp));

    // 4. 设置默认主题颜色 (逻辑坐标已横屏, 覆盖整个 296x240 区域)
    lvgl_port_lock(0);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);
    lvgl_port_unlock();

    s_ready = true;
    ESP_LOGI(TAG, "========== LVGL Init OK ==========");
    return ESP_OK;
}

/* ========== 获取就绪状态 ========== */

bool lvgl_is_ready(void)
{
    return s_ready;
}
