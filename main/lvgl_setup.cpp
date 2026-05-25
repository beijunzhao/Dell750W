/**
 * lvgl_setup.cpp - LVGL 显示初始化 + 色彩演示 (基于 esp_lvgl_port)
 *
 * 硬件平台: ESP32-C3
 * 屏幕: 2.01英寸 ST7789P3, 240x296, RGB565
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

    // 3. 添加 ST7789 显示设备
    // buffer_size: 40 行 × 240 像素 = 9600 像素 ≈ 19KB (单缓冲, ESP32-C3 DRAM)
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
    ESP_LOGI(TAG, "Display added: %dx%d, rotation=portrait", disp_cfg.hres, disp_cfg.vres);

    // 4. 设置默认主题颜色
    lvgl_port_lock(0);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, 0);
    lvgl_port_unlock();

    s_ready = true;
    ESP_LOGI(TAG, "========== LVGL Init OK ==========");
    return ESP_OK;
}

/* ========== 色彩演示 ========== */

void lvgl_demo_run(void)
{
    if (!s_ready) {
        ESP_LOGE(TAG, "LVGL not initialized, cannot run demo");
        return;
    }

    ESP_LOGI(TAG, "========== Demo Start ==========");

    // 阶段 1: 红 → 绿 → 蓝, 循环 3 次
    for (int cycle = 1; cycle <= 3; cycle++) {
        lvgl_port_lock(0);

        lv_obj_set_style_bg_color(lv_screen_active(),
                                  lv_color_make(255, 0, 0), 0);
        ESP_LOGI(TAG, "Cycle %d/3: RED", cycle);
        lvgl_port_unlock();
        vTaskDelay(pdMS_TO_TICKS(1000));

        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(lv_screen_active(),
                                  lv_color_make(0, 255, 0), 0);
        ESP_LOGI(TAG, "Cycle %d/3: GREEN", cycle);
        lvgl_port_unlock();
        vTaskDelay(pdMS_TO_TICKS(1000));

        lvgl_port_lock(0);
        lv_obj_set_style_bg_color(lv_screen_active(),
                                  lv_color_make(0, 0, 255), 0);
        ESP_LOGI(TAG, "Cycle %d/3: BLUE", cycle);
        lvgl_port_unlock();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // 阶段 2: 白底 + 中央黑色矩形框 + "Hello ESP32-C3"
    ESP_LOGI(TAG, "Drawing final screen: white bg + black frame + text");

    lvgl_port_lock(0);

    // 白色背景
    lv_obj_set_style_bg_color(lv_screen_active(),
                              lv_color_make(255, 255, 255), 0);

    // 中央黑色矩形框: 160×110, 边框 4px
    lv_obj_t *rect = lv_obj_create(lv_screen_active());
    lv_obj_set_size(rect, 160, 110);
    lv_obj_center(rect);
    lv_obj_set_style_radius(rect, 0, 0);                // 无圆角
    lv_obj_set_style_bg_color(rect, lv_color_make(255, 255, 255), 0);
    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(rect, lv_color_make(0, 0, 0), 0);
    lv_obj_set_style_border_width(rect, 4, 0);
    lv_obj_set_style_border_opa(rect, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(rect, 0, 0);

    // "Hello ESP32-C3" 文字
    lv_obj_t *label = lv_label_create(rect);
    lv_label_set_text(label, "Hello ESP32-C3");
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, lv_color_make(0, 0, 0), 0);

    lvgl_port_unlock();

    ESP_LOGI(TAG, "========== Demo Complete ==========");
}
