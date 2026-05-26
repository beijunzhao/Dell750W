/**
 * lvgl_ui.cpp - 电源监控仪表盘 UI (LVGL v9)
 *
 * 屏幕: ST7789P3 240x296 → LVGL 旋转 270° → 296x240 横屏
 *
 * 设计风格: 纯黑底 + 青蓝科技感 + 卡片式数据
 *
 * 布局 (横屏 296x240):
 *   ┌──────────────────────────────────────────┐
 *   │ ▌DELTA POWER                  ● ONLINE   │ ← 标题栏 (32px)
 *   │──────────────────────────────────────────│ ← 青色分隔线
 *   │                                          │
 *   │            1 2 . 0 5                     │ ← 电压大字 (28px)
 *   │            V O U T                       │ ← 单位标签
 *   │                                          │
 *   │  ┌──────────┐ ┌──────────┐ ┌──────────┐  │
 *   │  │  45.2    │ │  543     │ │  94.1    │  │ ← 数据卡片 (24px)
 *   │  │  电流 A  │ │  功率 W  │ │  效率 %  │  │
 *   │  └──────────┘ └──────────┘ └──────────┘  │
 *   │                                          │
 *   │──────────────────────────────────────────│ ← 青色分隔线
 *   │  INPUT 220V   TEMP 45°C   FAN 3200RPM   │ ← 底部状态 (14px)
 *   │  ● ADC OK                        v1.0   │
 *   └──────────────────────────────────────────┘
 */
#include "lvgl_ui.h"
#include "lvgl.h"
#include "pmbus.h"
#include "adc_sampler.h"
#include "power_control.h"
#include "pin_map.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LVGL_UI";

/* ========== LVGL 对象句柄 ========== */
static bool s_ui_ready = false;

/* 标题栏 */
static lv_obj_t *s_label_title  = NULL;
static lv_obj_t *s_label_status_dot = NULL;
static lv_obj_t *s_label_status_text = NULL;

/* 电压区 */
static lv_obj_t *s_label_voltage = NULL;
static lv_obj_t *s_label_v_unit  = NULL;

/* 三列数据卡片 */
static lv_obj_t *s_label_current_val = NULL;
static lv_obj_t *s_label_power_val   = NULL;
static lv_obj_t *s_label_eff_val     = NULL;

/* 底部状态 */
static lv_obj_t *s_label_input  = NULL;
static lv_obj_t *s_label_temp   = NULL;
static lv_obj_t *s_label_fan    = NULL;
static lv_obj_t *s_label_sys_ok = NULL;
static lv_obj_t *s_label_version = NULL;

/* 更新定时器 */
static lv_timer_t *s_update_timer = NULL;

/* ========== 颜色 (科技暗黑主题) ========== */
#define C_BLACK       lv_color_make(0x00, 0x00, 0x00)
#define C_WHITE       lv_color_make(0xFF, 0xFF, 0xFF)
#define C_ACCENT      lv_color_make(0x00, 0xBC, 0xD4)   // 青色 #00BCD4
#define C_ACCENT_DIM  lv_color_make(0x00, 0x6A, 0x7A)   // 暗青色
#define C_GREEN       lv_color_make(0x00, 0xE6, 0x76)   // 亮绿
#define C_RED         lv_color_make(0xFF, 0x17, 0x44)   // 亮红
#define C_ORANGE      lv_color_make(0xFF, 0x91, 0x00)   // 橙色
#define C_GRAY        lv_color_make(0x78, 0x90, 0x9C)   // 蓝灰
#define C_DIM         lv_color_make(0x45, 0x5A, 0x64)   // 暗蓝灰
#define C_CARD_BG     lv_color_make(0x08, 0x0E, 0x14)   // 卡片背景
#define C_CARD_BDR    lv_color_make(0x15, 0x1E, 0x2A)   // 卡片边框
#define C_LINE        lv_color_make(0x0E, 0x33, 0x3E)   // 分隔线 (暗青)

/* ========== 字体 (在 menuconfig 中启用) ========== */
#define FONT_TITLE  (&lv_font_montserrat_20)
#define FONT_VALUE  (&lv_font_montserrat_28)
#define FONT_CARD   (&lv_font_montserrat_24)
#define FONT_SMALL  (&lv_font_montserrat_14)

/* ========== 布局常量 ========== */
#define SCR_W       296
#define SCR_H       240
#define HEADER_H    33
#define CARD_W      90
#define CARD_H      88
#define CARD_Y      112
#define CARD_GAP    8
#define CARD_X0     5    // (296 - 3*90 - 2*8) / 2 = 5, 左右对称

/* ========== 前向声明 ========== */
static void _create_header(lv_obj_t *parent);
static void _create_accent_line(lv_obj_t *parent, int y);
static void _create_voltage_area(lv_obj_t *parent);
static lv_obj_t *_create_card(lv_obj_t *parent, int x, const char *title, const char *unit);
static void _create_data_cards(lv_obj_t *parent);
static void _create_status_bar(lv_obj_t *parent);
static void _update_timer_cb(lv_timer_t *timer);
static void _update_all(void);

/* ========== 公开 API ========== */

void lvgl_ui_init(void)
{
    if (s_ui_ready) {
        ESP_LOGW(TAG, "UI already initialized");
        return;
    }

    ESP_LOGI(TAG, "========== LVGL UI Init ==========");

    lv_obj_t *scr = lv_screen_active();

    // 纯黑背景
    lv_obj_set_style_bg_color(scr, C_BLACK, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    // 创建各区域
    _create_header(scr);         // 顶部标题栏
    _create_accent_line(scr, HEADER_H);  // 标题下方分隔线
    _create_voltage_area(scr);   // 中部电压大数字
    _create_data_cards(scr);     // 三列数据卡片 (y=112, h=88 → 底部=200)
    _create_accent_line(scr, 200);       // 卡片下方分隔线
    _create_status_bar(scr);     // 底部状态栏 (y=204 起)

    // 创建定时器: 每 500ms 更新
    s_update_timer = lv_timer_create(_update_timer_cb, 500, NULL);
    lv_timer_ready(s_update_timer);

    s_ui_ready = true;
    ESP_LOGI(TAG, "========== LVGL UI Ready ==========");
}

void lvgl_ui_deinit(void)
{
    if (!s_ui_ready) return;

    if (s_update_timer) {
        lv_timer_delete(s_update_timer);
        s_update_timer = NULL;
    }

    s_ui_ready = false;
    ESP_LOGI(TAG, "LVGL UI deinitialized");
}

/* ========== 顶部标题栏 ========== */

static void _create_header(lv_obj_t *parent)
{
    // 青色装饰条 (左侧)
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, 3, 20);
    lv_obj_set_pos(bar, 14, 8);
    lv_obj_set_style_bg_color(bar, C_ACCENT, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 1, 0);

    // 标题文字 "DELTA POWER"
    s_label_title = lv_label_create(parent);
    lv_label_set_text(s_label_title, "DELTA POWER");
    lv_obj_set_pos(s_label_title, 24, 5);
    lv_obj_set_style_text_color(s_label_title, C_WHITE, 0);
    lv_obj_set_style_text_font(s_label_title, FONT_TITLE, 0);

    // 状态指示灯 (右上角)
    s_label_status_dot = lv_label_create(parent);
    lv_label_set_text(s_label_status_dot, "●");
    lv_obj_set_pos(s_label_status_dot, 252, 10);
    lv_obj_set_style_text_color(s_label_status_dot, C_RED, 0);
    lv_obj_set_style_text_font(s_label_status_dot, FONT_SMALL, 0);

    // 状态文字
    s_label_status_text = lv_label_create(parent);
    lv_label_set_text(s_label_status_text, "OFF");
    lv_obj_set_pos(s_label_status_text, 264, 10);
    lv_obj_set_style_text_color(s_label_status_text, C_DIM, 0);
    lv_obj_set_style_text_font(s_label_status_text, FONT_SMALL, 0);
}

/* ========== 青色分隔线 ========== */

static void _create_accent_line(lv_obj_t *parent, int y)
{
    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_set_size(line, SCR_W, 1);
    lv_obj_set_pos(line, 0, y);
    lv_obj_set_style_bg_color(line, C_LINE, 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);
}

/* ========== 电压大数字区域 ========== */

static void _create_voltage_area(lv_obj_t *parent)
{
    // 电压数字 (巨大, 居中)
    s_label_voltage = lv_label_create(parent);
    lv_label_set_text(s_label_voltage, "0.00");
    lv_obj_set_pos(s_label_voltage, 0, 42);
    lv_obj_set_width(s_label_voltage, SCR_W);
    lv_obj_set_style_text_color(s_label_voltage, C_WHITE, 0);
    lv_obj_set_style_text_font(s_label_voltage, FONT_VALUE, 0);
    lv_obj_set_style_text_align(s_label_voltage, LV_TEXT_ALIGN_CENTER, 0);

    // 单位标签 (居中, 青色)
    s_label_v_unit = lv_label_create(parent);
    lv_label_set_text(s_label_v_unit, "VOLTS");
    lv_obj_set_pos(s_label_v_unit, 0, 80);
    lv_obj_set_width(s_label_v_unit, SCR_W);
    lv_obj_set_style_text_color(s_label_v_unit, C_ACCENT, 0);
    lv_obj_set_style_text_font(s_label_v_unit, FONT_SMALL, 0);
    lv_obj_set_style_text_align(s_label_v_unit, LV_TEXT_ALIGN_CENTER, 0);
}

/* ========== 数据卡片 ========== */

static lv_obj_t *_create_card(lv_obj_t *parent, int x,
                               const char *title, const char *unit)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_set_pos(card, x, CARD_Y);
    lv_obj_set_style_bg_color(card, C_CARD_BG, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, C_CARD_BDR, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 4, 0);
    lv_obj_set_style_pad_all(card, 0, 0);

    // 顶部青色装饰线
    lv_obj_t *top = lv_obj_create(card);
    lv_obj_set_size(top, CARD_W, 2);
    lv_obj_set_pos(top, 0, 0);
    lv_obj_set_style_bg_color(top, C_ACCENT, 0);
    lv_obj_set_style_bg_opa(top, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(top, 0, 0);
    lv_obj_set_style_radius(top, 0, 0);

    // 标题 (顶部)
    lv_obj_t *lbl = lv_label_create(card);
    lv_label_set_text(lbl, title);
    lv_obj_set_pos(lbl, 0, 10);
    lv_obj_set_width(lbl, CARD_W);
    lv_obj_set_style_text_color(lbl, C_DIM, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(lbl, FONT_SMALL, 0);

    // 数值 (中间, 占位, 由 _update_all 写入)
    lv_obj_t *val = lv_label_create(card);
    lv_label_set_text(val, "0");
    lv_obj_set_pos(val, 0, 32);
    lv_obj_set_width(val, CARD_W);
    lv_obj_set_style_text_color(val, C_WHITE, 0);
    lv_obj_set_style_text_align(val, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(val, FONT_CARD, 0);

    // 单位 (底部)
    lv_obj_t *unit_lbl = lv_label_create(card);
    lv_label_set_text(unit_lbl, unit);
    lv_obj_set_pos(unit_lbl, 0, 66);
    lv_obj_set_width(unit_lbl, CARD_W);
    lv_obj_set_style_text_color(unit_lbl, C_ACCENT_DIM, 0);
    lv_obj_set_style_text_align(unit_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(unit_lbl, FONT_SMALL, 0);

    (void)title;
    (void)unit;
    return val;  // 返回数值标签供更新
}

static void _create_data_cards(lv_obj_t *parent)
{
    int x1 = CARD_X0;
    int x2 = CARD_X0 + CARD_W + CARD_GAP;
    int x3 = CARD_X0 + (CARD_W + CARD_GAP) * 2;

    s_label_current_val = _create_card(parent, x1, "电流", "A");
    s_label_power_val   = _create_card(parent, x2, "功率", "W");
    s_label_eff_val     = _create_card(parent, x3, "效率", "%");
}

/* ========== 底部状态栏 ========== */

static void _create_status_bar(lv_obj_t *parent)
{
    int bar_y = 204;

    // 第1行: 输入电压 / 温度 / 风扇
    s_label_input = lv_label_create(parent);
    lv_label_set_text(s_label_input, "INPUT  --V");
    lv_obj_set_pos(s_label_input, 16, bar_y);
    lv_obj_set_style_text_color(s_label_input, C_GRAY, 0);
    lv_obj_set_style_text_font(s_label_input, FONT_SMALL, 0);

    s_label_temp = lv_label_create(parent);
    lv_label_set_text(s_label_temp, "TEMP  --°C");
    lv_obj_set_pos(s_label_temp, 120, bar_y);
    lv_obj_set_style_text_color(s_label_temp, C_GRAY, 0);
    lv_obj_set_style_text_font(s_label_temp, FONT_SMALL, 0);

    s_label_fan = lv_label_create(parent);
    lv_label_set_text(s_label_fan, "FAN  --RPM");
    lv_obj_set_pos(s_label_fan, 216, bar_y);
    lv_obj_set_style_text_color(s_label_fan, C_GRAY, 0);
    lv_obj_set_style_text_font(s_label_fan, FONT_SMALL, 0);

    // 第2行: 系统状态 + 版本号
    s_label_sys_ok = lv_label_create(parent);
    lv_label_set_text(s_label_sys_ok, "● ADC OK");
    lv_obj_set_pos(s_label_sys_ok, 16, bar_y + 20);
    lv_obj_set_style_text_color(s_label_sys_ok, C_GREEN, 0);
    lv_obj_set_style_text_font(s_label_sys_ok, FONT_SMALL, 0);

    s_label_version = lv_label_create(parent);
    lv_label_set_text(s_label_version, "v1.0");
    lv_obj_set_pos(s_label_version, 260, bar_y + 20);
    lv_obj_set_style_text_color(s_label_version, C_DIM, 0);
    lv_obj_set_style_text_font(s_label_version, FONT_SMALL, 0);
}

/* ========== 数据更新 ========== */

static void _update_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    _update_all();
}

static void _update_all(void)
{
    float v_out = ADCSampler::getVoltage();
    float i_out = PMBus::I_out;
    float w_out = PMBus::W_out;
    float v_in  = PMBus::V_in;
    float w_in  = PMBus::W_in;
    float temp1 = PMBus::temperature[0];
    float temp2 = PMBus::temperature[1];
    float fan   = PMBus::fanSpeed[0];
    bool  powerOn    = PowerControl::isPoweredOn();
    bool  deviceOk   = PMBus::isDeviceOnline();

    char buf[24];

    /* ---- 电源状态 ----
     * 优先级: 先看 PMBus 是否在线, 再看电源开关 */
    if (!deviceOk) {
        lv_label_set_text(s_label_status_dot, "●");
        lv_obj_set_style_text_color(s_label_status_dot, C_RED, 0);
        lv_label_set_text(s_label_status_text, "NO PSU");
        lv_obj_set_style_text_color(s_label_status_text, C_RED, 0);
    } else if (powerOn) {
        lv_label_set_text(s_label_status_dot, "●");
        lv_obj_set_style_text_color(s_label_status_dot, C_GREEN, 0);
        lv_label_set_text(s_label_status_text, "ON");
        lv_obj_set_style_text_color(s_label_status_text, C_GREEN, 0);
    } else {
        lv_label_set_text(s_label_status_dot, "●");
        lv_obj_set_style_text_color(s_label_status_dot, C_RED, 0);
        lv_label_set_text(s_label_status_text, "OFF");
        lv_obj_set_style_text_color(s_label_status_text, C_ORANGE, 0);
    }

    /* ---- 电压 (大字) ---- */
    snprintf(buf, sizeof(buf), "%.2f", v_out);
    lv_label_set_text(s_label_voltage, buf);

    /* ---- 电流 ---- */
    snprintf(buf, sizeof(buf), "%.1f", i_out);
    lv_label_set_text(s_label_current_val, buf);

    /* ---- 功率 ---- */
    if (w_out >= 100.0f) {
        snprintf(buf, sizeof(buf), "%.0f", w_out);
    } else if (w_out >= 10.0f) {
        snprintf(buf, sizeof(buf), "%.1f", w_out);
    } else {
        snprintf(buf, sizeof(buf), "%.2f", w_out);
    }
    lv_label_set_text(s_label_power_val, buf);

    /* ---- 效率 ---- */
    if (deviceOk && w_in > 0 && w_out >= 0) {
        float eff = (w_out / w_in) * 100.0f;
        if (eff > 100.0f) eff = 100.0f;
        snprintf(buf, sizeof(buf), "%.1f", eff);
        lv_label_set_text(s_label_eff_val, buf);
        if (eff > 90.0f) {
            lv_obj_set_style_text_color(s_label_eff_val, C_GREEN, 0);
        } else if (eff > 70.0f) {
            lv_obj_set_style_text_color(s_label_eff_val, C_ORANGE, 0);
        } else {
            lv_obj_set_style_text_color(s_label_eff_val, C_RED, 0);
        }
    } else {
        lv_label_set_text(s_label_eff_val, "--");
        lv_obj_set_style_text_color(s_label_eff_val, C_DIM, 0);
    }

    /* ---- 输入电压 ---- */
    if (deviceOk && v_in > 0) {
        snprintf(buf, sizeof(buf), "INPUT  %.0fV", v_in);
        lv_label_set_text(s_label_input, buf);
    } else {
        lv_label_set_text(s_label_input, "INPUT  --V");
    }

    /* ---- 温度 ---- */
    float maxTemp = (temp1 > temp2) ? temp1 : temp2;
    if (deviceOk && maxTemp > 0) {
        snprintf(buf, sizeof(buf), "TEMP  %.0f°C", maxTemp);
        lv_label_set_text(s_label_temp, buf);
        if (maxTemp > 80.0f) {
            lv_obj_set_style_text_color(s_label_temp, C_RED, 0);
        } else if (maxTemp > 60.0f) {
            lv_obj_set_style_text_color(s_label_temp, C_ORANGE, 0);
        } else {
            lv_obj_set_style_text_color(s_label_temp, C_GRAY, 0);
        }
    } else {
        lv_label_set_text(s_label_temp, "TEMP  --°C");
        lv_obj_set_style_text_color(s_label_temp, C_GRAY, 0);
    }

    /* ---- 风扇 ---- */
    if (deviceOk && fan > 0) {
        snprintf(buf, sizeof(buf), "FAN  %.0fRPM", fan);
        lv_label_set_text(s_label_fan, buf);
    } else {
        lv_label_set_text(s_label_fan, "FAN  --RPM");
    }

    /* ---- 系统状态 ---- */
    if (!deviceOk) {
        lv_label_set_text(s_label_sys_ok, "● PSU OFF");
        lv_obj_set_style_text_color(s_label_sys_ok, C_RED, 0);
    } else {
        lv_label_set_text(s_label_sys_ok, "● ADC OK");
        lv_obj_set_style_text_color(s_label_sys_ok, C_GREEN, 0);
    }
}
