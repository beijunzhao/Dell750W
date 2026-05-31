/**
 * lvgl_ui.h - 电源监控仪表盘 UI (LVGL v9)
 *
 * 屏幕: ST7789P3 240x296 → LVGL 旋转 270° → 296x240 横屏
 * 风格: 科技暗黑 + 多页面 + 底部导航
 *
 * 页面:
 *   0 - 仪表盘主页: 电压大字 + 弧形进度 + 功率/电流卡片
 *   1 - 详细数据: 输入/输出参数、温度、风扇
 *   2 - 系统信息: 设备状态、ADC、版本
 */
#ifndef LVGL_UI_H
#define LVGL_UI_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 LVGL 仪表盘 UI
 *
 * 创建所有 UI 控件并启动定时器刷新。
 * 必须在 lvgl_setup_init() 成功后调用。
 */
void lvgl_ui_init(void);

/**
 * @brief 销毁 UI 控件并停止定时器
 */
void lvgl_ui_deinit(void);

/**
 * @brief 颜色测试程序 - 在 LCD 上显示各种颜色色块和文字
 *
 * 用于确认 ST7789 实际显示的颜色是否与预期一致。
 * 调用此函数会替换掉正常的 UI。
 */
void lvgl_color_test_init(void);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_UI_H */
