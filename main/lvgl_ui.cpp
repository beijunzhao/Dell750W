/**
 * lvgl_ui.cpp - 电源监控仪表盘 UI (LVGL v9)
 *
 * 屏幕: ST7789P3 240x296 → LVGL 旋转 270° → 296x240 横屏
 *
 * 设计风格: 科技暗黑 + 手机App风格 + 单页极简
 *
 * 布局 (横屏 296x240):
 *   ┌──────────────────────────────────────────┐
 *   │  DELTA POWER                    ● ONLINE  │ ← 标题栏 (28px)
 *   │──────────────────────────────────────────│
 *   │  12.05                                    │
   │  VOLTS                                    │ ← 电压大数字 (28px)
 *   │                                          │
 *   │  ┌──────┐ ┌──────┐ ┌──────┐              │
 *   │  │ 5.2A │ │ 62W  │ │ 94%  │              │ ← 数据卡片 (24px)
 *   │  │ 电流  │ │ 功率  │ │ 效率  │              │
 *   │  └──────┘ └──────┘ └──────┘              │
 *   │                                          │
 *   │  INPUT 220V  TEMP 45°C  FAN 3200RPM      │ ← 底部状态 (14px)
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
static lv_obj_t *s_label_status = NULL;

/* 电压区 */
static lv_obj_t *s_label_voltage = NULL;
static lv_obj_t *s_label_v_unit  = NULL;

/* 三列数据卡片 */
static lv_obj_t *s_card_current_val = NULL;
static lv_obj_t *s_card_power_val   = NULL;
static lv_obj_t *s_card_eff_val     = NULL;

/* 底部状态 */
static lv_obj_t *s_label_input = NULL;
static lv_obj_t *s_label_temp  = NULL;
static lv_obj_t *s_label_fan   = NULL;

/* 更新定时器 */
static lv_timer_t *s_update_timer = NULL;

/* ========== 颜色 (科技暗黑主题) ========== */
#define C_BLACK       lv_color_make(0x00, 0x00, 0x00)
#define C_WHITE       lv_color_make(0xFF, 0xFF, 0xFF)
#define C_CYAN        lv_color_make(0x00, 0xE5, 0xFF)   /* 亮青 #00E5FF */
#define C_CYAN_DIM    lv_color_make(0x00, 0x80, 0x99)   /* 暗青 */
#define C_CYAN_DARK   lv_color_make(0x00, 0x33, 0x44)   /* 深青 */
#define C_GREEN       lv_color_make(0x00, 0xE6, 0x76)   /* 亮绿 */
#define C_RED         lv_color_make(0xFF, 0x17, 0x44)   /* 亮红 */
#define C_ORANGE      lv_color_make(0xFF, 0x91, 0x00)   /* 橙色 */
#define C_GOLD        lv_color_make(0xFF, 0xD5, 0x4F)   /* 金色 */
#define C_PURPLE      lv_color_make(0x7C, 0x4D, 0xFF)   /* 紫色 */
#define C_GRAY        lv_color_make(0x78, 0x90, 0x9C)   /* 蓝灰 */
#define C_DIM         lv_color_make(0x45, 0x5A, 0x64)   /* 暗蓝灰 */
#define C_CARD_BG     lv_color_make(0x08, 0x12, 0x1A)   /* 卡片背景 */
#define C_CARD_BDR    lv_color_make(0x0E, 0x25, 0x33)   /* 卡片边框 */

/* ========== 字体 ========== */
#define FONT_TITLE  (&lv_font_montserrat_20)
#define FONT_VOLT   (&lv_font_montserrat_28)
#define FONT_CARD   (&lv_font_montserrat_24)
#define FONT_SMALL  (&lv_font_montserrat_14)

/* ========== 布局常量 ========== */
#define SCR_W       296
#define SCR_H       240
#define HEADER_H    28
#define CARD_W      88
#define CARD_H      72
#define CARD_Y      108
#define CARD_GAP    8
#define CARD_X0     10

/* ========== 前向声明 ========== */
static void _create_header(lv_obj_t *parent);
static void _create_voltage_area(lv_obj_t *parent);
static lv_obj_t *_create_card(lv_obj_t *parent, int x, const char *title,
                              const char *unit, lv_color_t accent);
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

    /* 纯黑背景 */
    lv_obj_set_style_bg_color(scr, C_BLACK, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(scr, 0, 0);

    /* 创建各区域 */
    _create_header(scr);
    _create_voltage_area(scr);
    _create_data_cards(scr);
    _create_status_bar(scr);

    /* 创建定时器: 每 500ms 更新 */
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
    /* 标题栏背景 */
    lv_obj_t *hdr = lv_obj_create(parent);
    lv_obj_set_size(hdr, SCR_W, HEADER_H);
    lv_obj_set_pos(hdr, 0, 0);
    lv_obj_set_style_bg_color(hdr, C_BLACK, 0);
    lv_obj_set_style_bg_opa(hdr, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_radius(hdr, 0, 0);
    lv_obj_set_style_pad_all(hdr, 0, 0);

    /* 左侧青色装饰条 */
    lv_obj_t *bar = lv_obj_create(hdr);
    lv_obj_set_size(bar, 3, 16);
    lv_obj_set_pos(bar, 8, 6);
    lv_obj_set_style_bg_color(bar, C_CYAN, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 1, 0);

    /* 标题 */
    s_label_title = lv_label_create(hdr);
    lv_label_set_text(s_label_title, "DELTA POWER");
    lv_obj_set_pos(s_label_title, 18, 4);
    lv_obj_set_style_text_color(s_label_title, C_WHITE, 0);
    lv_obj_set_style_text_font(s_label_title, FONT_TITLE, 0);

    /* 右上角状态 */
    s_label_status = lv_label_create(hdr);
    lv_label_set_text(s_label_status, "● ONLINE");
    lv_obj_set_pos(s_label_status, 200, 5);
    lv_obj_set_style_text_color(s_label_status, C_GREEN, 0);
    lv_obj_set_style_text_font(s_label_status, FONT_SMALL, 0);

    /* 分隔线 */
    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_set_size(line, SCR_W, 1);
    lv_obj_set_pos(line, 0, HEADER_H);
    lv_obj_set_style_bg_color(line, C_CYAN_DARK, 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);
}

/* ========== 电压大数字区域 ========== */

static void _create_voltage_area(lv_obj_t *parent)
{
    /* 电压数字 (28px, 居中) */
    s_label_voltage = lv_label_create(parent);
    lv_label_set_text(s_label_voltage, "0.00");
    lv_obj_set_pos(s_label_voltage, 0, 38);
    lv_obj_set_width(s_label_voltage, SCR_W);
    lv_obj_set_style_text_color(s_label_voltage, C_WHITE, 0);
    lv_obj_set_style_text_font(s_label_voltage, FONT_VOLT, 0);
    lv_obj_set_style_text_align(s_label_voltage, LV_TEXT_ALIGN_CENTER, 0);

    /* VOLTS 标签 */
    s_label_v_unit = lv_label_create(parent);
    lv_label_set_text(s_label_v_unit, "VOLTS");
    lv_obj_set_pos(s_label_v_unit, 0, 76);
    lv_obj_set_width(s_label_v_unit, SCR_W);
    lv_obj_set_style_text_color(s_label_v_unit, C_CYAN_DIM, 0);
    lv_obj_set_style_text_font(s_label_v_unit, FONT_SMALL, 0);
    lv_obj_set_style_text_align(s_label_v_unit, LV_TEXT_ALIGN_CENTER, 0);
}

/* ========== 数据卡片 ========== */

static lv_obj_t *_create_card(lv_obj_t *parent, int x, const char *title,
                              const char *unit, lv_color_t accent)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, CARD_W, CARD_H);
    lv_obj_set_pos(card, x, CARD_Y);
    lv_obj_set_style_bg_color(card, C_CARD_BG, 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, C_CARD_BDR, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_pad_all(card, 0, 0);

    /* 顶部装饰条 (2px, 对应颜色) */
    lv_obj_t *top = lv_obj_create(card);
    lv_obj_set_size(top, CARD_W, 2);
    lv_obj_set_pos(top, 0, 0);
    lv_obj_set_style_bg_color(top, accent, 0);
    lv_obj_set_style_bg_opa(top, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(top, 0, 0);
    lv_obj_set_style_radius(top, 0, 0);

    /* 数值 (24px) */
    lv_obj_t *val = lv_label_create(card);
    lv_label_set_text(val, "0");
    lv_obj_set_pos(val, 0, 10);
    lv_obj_set_width(val, CARD_W);
    lv_obj_set_style_text_color(val, C_WHITE, 0);
    lv_obj_set_style_text_align(val, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(val, FONT_CARD, 0);

    /* 标题 */
    lv_obj_t *lbl = lv_label_create(card);
    lv_label_set_text(lbl, title);
    lv_obj_set_pos(lbl, 0, 44);
    lv_obj_set_width(lbl, CARD_W);
    lv_obj_set_style_text_color(lbl, C_DIM, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(lbl, FONT_SMALL, 0);

    /* 单位 */
    lv_obj_t *unit_lbl = lv_label_create(card);
    lv_label_set_text(unit_lbl, unit);
    lv_obj_set_pos(unit_lbl, 0, 58);
    lv_obj_set_width(unit_lbl, CARD_W);
    lv_obj_set_style_text_color(unit_lbl, accent, 0);
    lv_obj_set_style_text_align(unit_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(unit_lbl, FONT_SMALL, 0);

    (void)lbl;
    (void)unit_lbl;
    return val;
}

static void _create_data_cards(lv_obj_t *parent)
{
    int x1 = CARD_X0;
    int x2 = CARD_X0 + CARD_W + CARD_GAP;
    int x3 = CARD_X0 + (CARD_W + CARD_GAP) * 2;

    s_card_current_val = _create_card(parent, x1, "电流", "A", C_CYAN);
    s_card_power_val   = _create_card(parent, x2, "功率", "W", C_PURPLE);
    s_card_eff_val     = _create_card(parent, x3, "效率", "%", C_GOLD);
}

/* ========== 底部状态栏 ========== */

static void _create_status_bar(lv_obj_t *parent)
{
    int bar_y = 195;

    /* 分隔线 */
    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_set_size(line, SCR_W - 20, 1);
    lv_obj_set_pos(line, 10, bar_y);
    lv_obj_set_style_bg_color(line, C_CYAN_DARK, 0);
    lv_obj_set_style_bg_opa(line, 80, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);

    /* 输入电压 */
    s_label_input = lv_label_create(parent);
    lv_label_set_text(s_label_input, "IN: --V");
    lv_obj_set_pos(s_label_input, 12, bar_y + 8);
    lv_obj_set_style_text_color(s_label_input, C_GRAY, 0);
    lv_obj_set_style_text_font(s_label_input, FONT_SMALL, 0);

    /* 温度 */
    s_label_temp = lv_label_create(parent);
    lv_label_set_text(s_label_temp, "TEMP: --°C");
    lv_obj_set_pos(s_label_temp, 108, bar_y + 8);
    lv_obj_set_style_text_color(s_label_temp, C_GRAY, 0);
    lv_obj_set_style_text_font(s_label_temp, FONT_SMALL, 0);

    /* 风扇 */
    s_label_fan = lv_label_create(parent);
    lv_label_set_text(s_label_fan, "FAN: --RPM");
    lv_obj_set_pos(s_label_fan, 204, bar_y + 8);
    lv_obj_set_style_text_color(s_label_fan, C_GRAY, 0);
    lv_obj_set_style_text_font(s_label_fan, FONT_SMALL, 0);
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
    bool  powerOn  = PowerControl::isPoweredOn();
    bool  deviceOk = PMBus::isDeviceOnline();

    char buf[24];

    /* ---- 标题栏状态 ---- */
    if (!deviceOk) {
        lv_label_set_text(s_label_status, "● NO PSU");
        lv_obj_set_style_text_color(s_label_status, C_RED, 0);
    } else if (powerOn) {
        lv_label_set_text(s_label_status, "● ONLINE");
        lv_obj_set_style_text_color(s_label_status, C_GREEN, 0);
    } else {
        lv_label_set_text(s_label_status, "● STANDBY");
        lv_obj_set_style_text_color(s_label_status, C_ORANGE, 0);
    }

    /* ---- 电压 (大字) ---- */
    snprintf(buf, sizeof(buf), "%.2f", v_out);
    lv_label_set_text(s_label_voltage, buf);

    /* ---- 电流 ---- */
    snprintf(buf, sizeof(buf), "%.1f", i_out);
    lv_label_set_text(s_card_current_val, buf);

    /* ---- 功率 ---- */
    if (w_out >= 100.0f) {
        snprintf(buf, sizeof(buf), "%.0f", w_out);
    } else if (w_out >= 10.0f) {
        snprintf(buf, sizeof(buf), "%.1f", w_out);
    } else {
        snprintf(buf, sizeof(buf), "%.2f", w_out);
    }
    lv_label_set_text(s_card_power_val, buf);

    /* ---- 效率 ---- */
    if (deviceOk && w_in > 0 && w_out >= 0) {
        float eff = (w_out / w_in) * 100.0f;
        if (eff > 100.0f) eff = 100.0f;
        snprintf(buf, sizeof(buf), "%.1f", eff);
        lv_label_set_text(s_card_eff_val, buf);
        if (eff > 90.0f) {
            lv_obj_set_style_text_color(s_card_eff_val, C_GREEN, 0);
        } else if (eff > 70.0f) {
            lv_obj_set_style_text_color(s_card_eff_val, C_ORANGE, 0);
        } else {
            lv_obj_set_style_text_color(s_card_eff_val, C_RED, 0);
        }
    } else {
        lv_label_set_text(s_card_eff_val, "--");
        lv_obj_set_style_text_color(s_card_eff_val, C_DIM, 0);
    }

    /* ---- 输入电压 ---- */
    if (deviceOk && v_in > 0) {
        snprintf(buf, sizeof(buf), "IN: %.0fV", v_in);
    } else {
        snprintf(buf, sizeof(buf), "IN: --V");
    }
    lv_label_set_text(s_label_input, buf);

    /* ---- 温度 ---- */
    float maxTemp = (temp1 > temp2) ? temp1 : temp2;
    if (deviceOk && maxTemp > 0) {
        snprintf(buf, sizeof(buf), "TEMP: %.0f°C", maxTemp);
        lv_label_set_text(s_label_temp, buf);
        if (maxTemp > 80.0f) {
            lv_obj_set_style_text_color(s_label_temp, C_RED, 0);
        } else if (maxTemp > 60.0f) {
            lv_obj_set_style_text_color(s_label_temp, C_ORANGE, 0);
        } else {
            lv_obj_set_style_text_color(s_label_temp, C_GRAY, 0);
        }
    } else {
        lv_label_set_text(s_label_temp, "TEMP: --°C");
        lv_obj_set_style_text_color(s_label_temp, C_GRAY, 0);
    }

    /* ---- 风扇 ---- */
    if (deviceOk && fan > 0) {
        snprintf(buf, sizeof(buf), "FAN: %.0fRPM", fan);
    } else {
        snprintf(buf, sizeof(buf), "FAN: --RPM");
    }
    lv_label_set_text(s_label_fan, buf);
}
