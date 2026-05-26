/**
 * lvgl_color_test.cpp - LVGL 颜色测试程序
 *
 * 在 LCD 上显示各种颜色的色块和文字，用于确认实际显示颜色。
 */
#include "lvgl_ui.h"
#include "lvgl.h"
#include "esp_log.h"

static const char *TAG = "COLOR_TEST";
static bool s_ready = false;

/* 辅助: 创建一个色块+文字行 */
static int _add_color_row(lv_obj_t *parent, int y,
                          const char *name,
                          uint8_t r, uint8_t g, uint8_t b)
{
    /* 色块 */
    lv_obj_t *rect = lv_obj_create(parent);
    lv_obj_set_size(rect, 30, 18);
    lv_obj_set_pos(rect, 10, y);
    lv_obj_set_style_bg_color(rect, lv_color_make(r, g, b), 0);
    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(rect, 1, 0);
    lv_obj_set_style_border_color(rect, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_radius(rect, 2, 0);

    /* 文字 (用同样的颜色显示) */
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, name);
    lv_obj_set_pos(lbl, 48, y);
    lv_obj_set_style_text_color(lbl, lv_color_make(r, g, b), 0);

    return y + 22;
}

void lvgl_color_test_init(void)
{
    if (s_ready) return;

    ESP_LOGI(TAG, "========== Color Test Init ==========");

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* 标题 */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "LVGL Color Test");
    lv_obj_set_pos(title, 10, 5);
    lv_obj_set_style_text_color(title, lv_color_make(0xFF, 0xFF, 0xFF), 0);

    int y = 30;

    y = _add_color_row(scr, y, "Black #000000",  0x00, 0x00, 0x00);
    y = _add_color_row(scr, y, "White #FFFFFF",  0xFF, 0xFF, 0xFF);
    y = _add_color_row(scr, y, "Red   #FF0000",  0xFF, 0x00, 0x00);
    y = _add_color_row(scr, y, "Green #00FF00",  0x00, 0xFF, 0x00);
    y = _add_color_row(scr, y, "Blue  #0000FF",  0x00, 0x00, 0xFF);
    y = _add_color_row(scr, y, "Cyan  #00BCD4",  0x00, 0xBC, 0xD4);
    y = _add_color_row(scr, y, "Gold  #FFD54F",  0xFF, 0xD5, 0x4F);
    y = _add_color_row(scr, y, "Sky   #4FD5FF",  0x4F, 0xD5, 0xFF);
    y = _add_color_row(scr, y, "Purple#7C4DFF",  0x7C, 0x4D, 0xFF);
    y = _add_color_row(scr, y, "Orange#FF9800",  0xFF, 0x98, 0x00);
    y = _add_color_row(scr, y, "Gray  #666666",  0x66, 0x66, 0x66);
    y = _add_color_row(scr, y, "DkGray#444444",  0x44, 0x44, 0x44);
    y = _add_color_row(scr, y, "Dim   #222222",  0x22, 0x22, 0x22);
    y = _add_color_row(scr, y, "Navy  #0F0F23",  0x0F, 0x0F, 0x23);
    y = _add_color_row(scr, y, "Card  #1A1A3E",  0x1A, 0x1A, 0x3E);
    y = _add_color_row(scr, y, "Card2 #111111",  0x11, 0x11, 0x11);

    /* 底部提示 */
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Tell me which color is wrong");
    lv_obj_set_pos(hint, 10, y + 10);
    lv_obj_set_style_text_color(hint, lv_color_make(0x66, 0x66, 0x66), 0);

    s_ready = true;
    ESP_LOGI(TAG, "========== Color Test Ready ==========");
}
