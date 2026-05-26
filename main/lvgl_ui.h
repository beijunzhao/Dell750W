/**
 * lvgl_ui.h - 电源监控仪表盘 UI (LVGL v9)
 *
 * 屏幕: ST7789P3 240x296 → LVGL 旋转 270° → 296x240 横屏
 * 风格: 纯黑底 + 青色科技感 + 卡片数据
 *
 * 布局 (296x240):
 *   - 标题栏: 装饰条 + "DELTA POWER" + 电源状态指示灯 (33px)
 *   - 电压大数: 28px 字体居中, 青色 "VOLTS" 标签 (42-100px)
 *   - 数据卡片: 电流/功率/效率 三列, 青色顶部装饰 (112-200px)
 *   - 底部状态: 输入电压/温度/风扇 + 系统状态/版本 (208-238px)
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
