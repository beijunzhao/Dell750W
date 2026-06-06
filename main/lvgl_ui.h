/**
 * lvgl_ui.h - 电源监控仪表盘 UI (LVGL v9)
 *
 * 屏幕: ST7789P3 240x296 → LVGL 旋转 270° → 296x240 横屏
 *
 * UI 由 NXP GUI Guider 设计, 生成文件在 DELL_LVGL/generated/
 * 此文件负责对接 GUI Guider UI 与项目数据源
 */
#ifndef LVGL_UI_H
#define LVGL_UI_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 按键类型 */
typedef enum {
    LVGL_KEY_UP,
    LVGL_KEY_DOWN,
    LVGL_KEY_OK,
    LVGL_KEY_OK_LONG,
} lvgl_key_t;

/**
 * @brief 初始化 LVGL 仪表盘 UI
 */
void lvgl_ui_init(void);

/**
 * @brief 销毁 UI 控件并停止定时器
 */
void lvgl_ui_deinit(void);

/**
 * @brief 检查 UI 是否已初始化就绪
 */
bool lvgl_ui_is_ready(void);

/**
 * @brief 按键处理入口（由 main.cpp 调用）
 * @param key 按键类型
 */
void lvgl_ui_handle_key(lvgl_key_t key);

/**
 * @brief 颜色测试程序
 */
void lvgl_color_test_init(void);

/**
 * @brief 重置去抖计时器（由 main.cpp 实现）
 */
void lvgl_reset_debounce(void);

/**
 * @brief 切换到校准页面（由 calibration.cpp 调用）
 */
void lvgl_show_calibration(void);

/**
 * @brief 切换到电流校准页面
 */
void lvgl_show_current_calibration(void);

/**
 * @brief 隐藏校准页面返回主页面（由 calibration.cpp 调用）
 */
void lvgl_hide_calibration(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_UI_H */