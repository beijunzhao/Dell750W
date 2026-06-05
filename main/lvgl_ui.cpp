/**
 * lvgl_ui.cpp - 电源监控仪表盘 UI (LVGL v9)
 *
 * 屏幕: ST7789P3 240x296 → LVGL 旋转 270° → 296x240 横屏
 *
 * UI 由 NXP GUI Guider 设计, 生成文件在 DELL_LVGL/generated/
 * 此文件负责对接 GUI Guider UI 与项目数据源
 */
#include "lvgl_ui.h"
#include "lvgl.h"
#include "pmbus.h"
#include "adc_sampler.h"
#include "power_control.h"
#include "pin_map.h"
#include "calibration.h"

#include "gui_guider.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LVGL_UI";
lv_ui guider_ui;

static bool s_ui_ready = false;
static lv_timer_t *s_update_timer = NULL;

/* ========== 运行时间 ========== */
static uint32_t s_run_sec = 0;
static uint32_t s_run_tick = 0;

static inline uint32_t _now_sec(void) {
    return (uint32_t)(esp_timer_get_time() / 1000000ULL);
}
static void _run_stop(void) {
    if (s_run_tick > 0) { s_run_sec += _now_sec() - s_run_tick; s_run_tick = 0; }
}
static void _run_start(void) { s_run_tick = _now_sec(); }
static uint32_t _run_total(void) {
    uint32_t t = s_run_sec;
    if (s_run_tick > 0) t += _now_sec() - s_run_tick;
    return t;
}

/* ========== 页面管理 ========== */
typedef enum { PAGE_MAIN = 0, PAGE_MENU } page_t;
static page_t s_page = PAGE_MAIN;

/* ========== Page 1 焦点导航 ========== */
#define FOCUS_COUNT 3
typedef enum { FOCUS_POWER = 0, FOCUS_VSET, FOCUS_ISET } focus_t;
static int s_focus = FOCUS_POWER;

/* ========== Page 2 菜单 ========== */
#define MENU_COUNT 5
static int s_menu_idx = 0;  /* 0~4 */

/* ========== 编辑模式 ========== */
static uint32_t s_last_ok_ms = 0;
#define DOUBLE_CLICK_MS 400
typedef enum { EDIT_NONE, EDIT_V, EDIT_I } edit_t;
static edit_t s_edit = EDIT_NONE;
static int s_epos = 0;
static bool s_blink = true;

/* ========== 前向声明 ========== */
static void _switch_to_page(page_t page);
static void _apply_focus(void);
static void _clear_focus(void);
static void _apply_menu_cursor(void);
static void _clear_menu_cursor(void);
static void _enter_edit(edit_t mode);
static void _exit_edit(void);
static void _ed_up(void);
static void _ed_dn(void);
static void _ed_ok(void);
static void _ed_show(void);
static void _digits(float v, int d[4]);
static float _value(const int d[4]);

/* 三角形光标对象 - 在 menu 页创建一个 label 用作 ">" 光标 */
static lv_obj_t *s_menu_cursor = NULL;

/* ========== 公开 API ========== */

void lvgl_ui_init(void)
{
    if (s_ui_ready) return;
    ESP_LOGI(TAG, "========== LVGL UI Init ==========");

    /* 先创建两个页面 */
    setup_ui(&guider_ui);
    setup_scr_screen_1(&guider_ui);  /* 创建 page 2 (screen_1)，默认隐藏 */

    /* 创建菜单光标 (在 screen_1 上) */
    s_menu_cursor = lv_label_create(guider_ui.screen_1);
    lv_obj_set_pos(s_menu_cursor, 35, 28);
    lv_obj_set_size(s_menu_cursor, 20, 24);
    lv_label_set_text(s_menu_cursor, ">");
    lv_obj_set_style_text_color(s_menu_cursor, lv_color_hex(0xFFFF00), (uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(s_menu_cursor, &lv_font_SourceHanSerifSC_Regular_24, (uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(s_menu_cursor, 255, (uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);

    /* 不让 lv_screen_load 自动删除旧 screen */
    guider_ui.screen_del = false;
    guider_ui.screen_1_del = false;

    /* 默认显示主页 */
    s_page = PAGE_MAIN;
    lv_screen_load(guider_ui.screen);

    s_update_timer = lv_timer_create([](lv_timer_t *t) {
        (void)t; if (!s_ui_ready) return;
        lv_ui *u = &guider_ui;
        char b[24];

        if (s_page == PAGE_MAIN) {
            if (s_edit != EDIT_NONE) s_blink = !s_blink;

            /* 电压/电流/功率 */
            { float v = ADCSampler::getVoltage(); int ip=(int)v,fp=(int)((v-ip)*100+0.5f); if(fp>=100){ip++;fp=0;} snprintf(b,sizeof(b),"%02d.%02d",ip,fp); lv_label_set_text(u->screen_label_49,b); }
            { float v = PMBus::I_out; int ip=(int)v,fp=(int)((v-ip)*100+0.5f); if(fp>=100){ip++;fp=0;} snprintf(b,sizeof(b),"%02d.%02d",ip,fp); lv_label_set_text(u->screen_label_33,b); }
            { float v = PMBus::W_out; if(v>=100) snprintf(b,sizeof(b),"%.0f",v); else if(v>=10) snprintf(b,sizeof(b),"%.1f",v); else{int ip=(int)v,fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;} snprintf(b,sizeof(b),"%02d.%02d",ip,fp);} lv_label_set_text(u->screen_label_34,b); }

            /* 电压/电流设置 */
            if (u->screen_label_52) {
                if (s_edit == EDIT_V) _ed_show();
                else { float v=PowerControl::getSetVoltage(); int ip=(int)v,fp=(int)((v-ip)*100+0.5f); if(fp>=100){ip++;fp=0;} snprintf(b,sizeof(b),"%02d.%02d",ip,fp); lv_label_set_text(u->screen_label_52,b); }
            }
            if (u->screen_label_54) {
                if (s_edit == EDIT_I) _ed_show();
                else { float v=PowerControl::getSetCurrent(); int ip=(int)v,fp=(int)((v-ip)*100+0.5f); if(fp>=100){ip++;fp=0;} snprintf(b,sizeof(b),"%02d.%02d",ip,fp); lv_label_set_text(u->screen_label_54,b); }
            }

            /* 电源按钮 */
            if (u->screen_btn_1_label) {
                if (PowerControl::isPoweredOn()) {
                    lv_label_set_text(u->screen_btn_1_label, "ON");
                    lv_obj_set_style_bg_color(u->screen_btn_1, lv_color_hex(0x00E676), (uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
                } else {
                    lv_label_set_text(u->screen_btn_1_label, "OFF");
                    lv_obj_set_style_bg_color(u->screen_btn_1, lv_color_hex(0xFF1744), (uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
                }
            }

            /* 能量 */
            if (u->screen_label_46) { float wh=PMBus::E_out,v=ADCSampler::getVoltage(); float ah=(v>1)?(wh/v):0; snprintf(b,sizeof(b),"%06.2f",ah); lv_label_set_text(u->screen_label_46,b); }
            if (u->screen_label_45) { float e=PMBus::E_out; snprintf(b,sizeof(b),"%06.2f",e); lv_label_set_text(u->screen_label_45,b); }

            /* 运行时间 */
            if (u->screen_label_51) {
                uint32_t t_ = _run_total();
                snprintf(b,sizeof(b),"%02d:%02d:%02d", (int)(t_/3600), (int)((t_%3600)/60), (int)(t_%60));
                lv_label_set_text(u->screen_label_51,b);
            }

            /* 风扇温度 */
            if (u->screen_label_42) { snprintf(b,sizeof(b),"%.0f",PMBus::fanSpeed[0]); lv_label_set_text(u->screen_label_42,b); }
            if (u->screen_label_43) { snprintf(b,sizeof(b),"%.0f",PMBus::temperature[0]); lv_label_set_text(u->screen_label_43,b); }
            if (u->screen_label_44) { snprintf(b,sizeof(b),"%.0f",PMBus::temperature[1]); lv_label_set_text(u->screen_label_44,b); }
        }
    }, 500, NULL);
    lv_timer_ready(s_update_timer);

    s_focus = FOCUS_POWER; s_edit = EDIT_NONE;
    s_run_sec = 0; s_run_tick = 0;
    if (PowerControl::isPoweredOn()) s_run_tick = _now_sec();

    _apply_focus();
    s_ui_ready = true;
    ESP_LOGI(TAG, "========== LVGL UI Ready ==========");
}

void lvgl_ui_deinit(void) { if(!s_ui_ready)return; if(s_update_timer){lv_timer_delete(s_update_timer);s_update_timer=NULL;} s_ui_ready=false; }
bool lvgl_ui_is_ready(void) { return s_ui_ready; }

/* ========== 页面切换 ========== */

/* main.cpp 中定义的全局去抖重置函数 */
void lvgl_reset_debounce(void);

static void _switch_to_page(page_t page)
{
    lvgl_reset_debounce();  /* 重置 main.cpp 中的去抖计时器 */

    if (page == PAGE_MAIN) {
        lv_screen_load(guider_ui.screen);
        s_page = PAGE_MAIN;
        _apply_focus();
    } else {
        lv_screen_load(guider_ui.screen_1);
        s_page = PAGE_MENU;
        s_menu_idx = 0;
        _apply_menu_cursor();
    }
}

/* ========== 按键处理 ========== */

void lvgl_ui_handle_key(lvgl_key_t key)
{
    if (!s_ui_ready) return;

    if (s_page == PAGE_MAIN) {
        /* === 主页 === */
        if (s_edit != EDIT_NONE) {
            switch (key) {
            case LVGL_KEY_UP: _ed_dn(); break;
            case LVGL_KEY_DOWN: _ed_up(); break;
            case LVGL_KEY_OK: _ed_ok(); break;
            case LVGL_KEY_OK_LONG: _exit_edit(); break;
            }
            return;
        }

        switch (key) {
        case LVGL_KEY_UP:
            s_focus = (s_focus + 1) % FOCUS_COUNT;
            _apply_focus();
            break;
        case LVGL_KEY_DOWN:
            s_focus = (s_focus + FOCUS_COUNT - 1) % FOCUS_COUNT;
            _apply_focus();
            break;
        case LVGL_KEY_OK:
            switch (s_focus) {
            case FOCUS_POWER:
                if (PowerControl::isPoweredOn()) { PowerControl::powerOff(); _run_stop(); }
                else { PowerControl::powerOn(); _run_start(); }
                break;
            case FOCUS_VSET: _enter_edit(EDIT_V); break;
            case FOCUS_ISET: _enter_edit(EDIT_I); break;
            }
            break;
        case LVGL_KEY_OK_LONG:
            /* 长按 OK → 进入菜单页 */
            _switch_to_page(PAGE_MENU);
            break;
        }
    } else {
        /* === 菜单页 === */
        switch (key) {
        case LVGL_KEY_UP:
            s_menu_idx = (s_menu_idx + MENU_COUNT - 1) % MENU_COUNT;  /* 上移 */
            _apply_menu_cursor();
            break;
        case LVGL_KEY_DOWN:
            s_menu_idx = (s_menu_idx + 1) % MENU_COUNT;  /* 下移 */
            _apply_menu_cursor();
            break;
        case LVGL_KEY_OK:
            if (s_menu_idx == 4) {
                _switch_to_page(PAGE_MAIN);  /* 返回首页 */
            } else {
                ESP_LOGI(TAG, "Menu %d selected", s_menu_idx);
            }
            break;
        case LVGL_KEY_OK_LONG:
            /* 长按 OK → 返回主页 */
            _switch_to_page(PAGE_MAIN);
            break;
        }
    }
}

/* ========== Page1 焦点样式 ========== */

static void _apply_focus(void)
{
    lv_ui *u = &guider_ui; if(!u->screen) return;
    _clear_focus();
    switch (s_focus) {
    case FOCUS_POWER:
        if(u->screen_btn_1) { lv_obj_set_style_border_width(u->screen_btn_1,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_color(u->screen_btn_1,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_opa(u->screen_btn_1,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); }
        break;
    case FOCUS_VSET:
        if(u->screen_label_23) { lv_obj_set_style_border_width(u->screen_label_23,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_color(u->screen_label_23,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_opa(u->screen_label_23,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_bg_color(u->screen_label_23,lv_color_hex(0x1A3344),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); }
        if(u->screen_label_52) lv_obj_set_style_text_color(u->screen_label_52,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
        break;
    case FOCUS_ISET:
        if(u->screen_label_28) { lv_obj_set_style_border_width(u->screen_label_28,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_color(u->screen_label_28,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_opa(u->screen_label_28,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_bg_color(u->screen_label_28,lv_color_hex(0x1A3344),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); }
        if(u->screen_label_54) lv_obj_set_style_text_color(u->screen_label_54,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
        break;
    }
}

static void _clear_focus(void)
{
    lv_ui *u = &guider_ui; if(!u->screen) return;
    if(u->screen_btn_1) lv_obj_set_style_border_width(u->screen_btn_1,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(u->screen_label_23){ lv_obj_set_style_border_width(u->screen_label_23,4,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_color(u->screen_label_23,lv_color_hex(0x2195f6),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_opa(u->screen_label_23,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); }
    if(u->screen_label_28){ lv_obj_set_style_border_width(u->screen_label_28,4,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_color(u->screen_label_28,lv_color_hex(0x2195f6),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_opa(u->screen_label_28,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); }
    if(u->screen_label_52) lv_obj_set_style_text_color(u->screen_label_52,lv_color_hex(0x00FF55),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(u->screen_label_54) lv_obj_set_style_text_color(u->screen_label_54,lv_color_hex(0x00FF55),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(u->screen_label_23) lv_obj_set_style_bg_color(u->screen_label_23,lv_color_hex(0x2194f4),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(u->screen_label_28) lv_obj_set_style_bg_color(u->screen_label_28,lv_color_hex(0x2194f4),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
}

/* ========== Page2 菜单光标 ========== */

/* 5 个菜单项的 Y 坐标 (参考 setup_scr_screen_1.c) */
static const int MENU_Y[] = { 19, 60, 97, 137, 178 };

static void _apply_menu_cursor(void)
{
    if (!s_menu_cursor) return;
    /* 移动到当前选项旁边 (Y 坐标偏移 2px 对齐) */
    lv_obj_set_pos(s_menu_cursor, 35, MENU_Y[s_menu_idx]);
    lv_obj_move_foreground(s_menu_cursor);
    lv_obj_clear_flag(s_menu_cursor, LV_OBJ_FLAG_HIDDEN);
}

static void _clear_menu_cursor(void)
{
    if (s_menu_cursor) lv_obj_add_flag(s_menu_cursor, LV_OBJ_FLAG_HIDDEN);
}

/* ========== 编辑模式 ========== */

static void _digits(float v, int d[4]) {
    int ip=(int)v; if(ip<0)ip=0; if(ip>99)ip=99;
    int fp=(int)((v-ip)*100+0.5f); if(fp>=100){ip++;fp=0;} if(fp<0)fp=0;
    d[0]=ip/10; d[1]=ip%10; d[2]=fp/10; d[3]=fp%10;
}

static float _value(const int d[4]) { return (float)(d[0]*10+d[1]) + (float)(d[2]*10+d[3])/100.0f; }

static void _enter_edit(edit_t mode) {
    s_edit = mode; s_epos = 0; s_blink = true;
    lv_ui *u = &guider_ui;
    if(mode==EDIT_V && u->screen_label_23) { lv_obj_set_style_border_width(u->screen_label_23,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_color(u->screen_label_23,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_opa(u->screen_label_23,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_bg_color(u->screen_label_23,lv_color_hex(0x331144),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); }
    if(mode==EDIT_V && u->screen_label_52) lv_obj_set_style_text_color(u->screen_label_52,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(mode==EDIT_I && u->screen_label_28) { lv_obj_set_style_border_width(u->screen_label_28,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_color(u->screen_label_28,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_border_opa(u->screen_label_28,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); lv_obj_set_style_bg_color(u->screen_label_28,lv_color_hex(0x331144),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT); }
    if(mode==EDIT_I && u->screen_label_54) lv_obj_set_style_text_color(u->screen_label_54,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
}

static void _exit_edit(void) { if(s_edit==EDIT_NONE)return; s_edit=EDIT_NONE; _apply_focus(); }

static void _ed_show(void) {
    lv_ui *u = &guider_ui; char b[8]; int d[4];
    _digits((s_edit==EDIT_V)?PowerControl::getSetVoltage():PowerControl::getSetCurrent(), d);
    int bk = !s_blink;
    char c0=(s_epos==0&&bk)?'_':('0'+d[0]), c1=(s_epos==1&&bk)?'_':('0'+d[1]);
    char c2=(s_epos==2&&bk)?'_':('0'+d[2]), c3=(s_epos==3&&bk)?'_':('0'+d[3]);
    snprintf(b,sizeof(b),"%c%c.%c%c",c0,c1,c2,c3);
    if(s_edit==EDIT_V && u->screen_label_52) lv_label_set_text(u->screen_label_52,b);
    if(s_edit==EDIT_I && u->screen_label_54) lv_label_set_text(u->screen_label_54,b);
}

static void _ed_up(void) {
    int d[4];
    if(s_edit==EDIT_V) { _digits(PowerControl::getSetVoltage(),d); d[s_epos]=(d[s_epos]+1)%10; float nv=_value(d); if(nv<=PowerControl::getVMax())PowerControl::setVoltage(nv); }
    if(s_edit==EDIT_I) { _digits(PowerControl::getSetCurrent(),d); d[s_epos]=(d[s_epos]+1)%10; float nv=_value(d); if(nv<=PowerControl::getIMax())PowerControl::setCurrent(nv); }
}

static void _ed_dn(void) {
    int d[4];
    if(s_edit==EDIT_V) { _digits(PowerControl::getSetVoltage(),d); d[s_epos]=(d[s_epos]+9)%10; float nv=_value(d); if(nv<=PowerControl::getVMax())PowerControl::setVoltage(nv); }
    if(s_edit==EDIT_I) { _digits(PowerControl::getSetCurrent(),d); d[s_epos]=(d[s_epos]+9)%10; float nv=_value(d); if(nv<=PowerControl::getIMax())PowerControl::setCurrent(nv); }
}

static void _ed_ok(void) {
    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    if (s_last_ok_ms != 0 && (now_ms - s_last_ok_ms) < DOUBLE_CLICK_MS) {
        s_last_ok_ms = 0; _exit_edit(); return;
    }
    s_last_ok_ms = now_ms;
    if (s_epos >= 3) return;
    s_epos++;
}