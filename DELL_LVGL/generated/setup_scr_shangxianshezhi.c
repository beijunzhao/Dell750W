/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_shangxianshezhi(lv_ui *ui)
{
    //Write codes shangxianshezhi
    ui->shangxianshezhi = lv_obj_create(NULL);
    lv_obj_set_size(ui->shangxianshezhi, 296, 240);
    lv_obj_set_scrollbar_mode(ui->shangxianshezhi, LV_SCROLLBAR_MODE_OFF);

    //Write style for shangxianshezhi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->shangxianshezhi, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->shangxianshezhi, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->shangxianshezhi, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_cont_1
    ui->shangxianshezhi_cont_1 = lv_obj_create(ui->shangxianshezhi);
    lv_obj_set_pos(ui->shangxianshezhi_cont_1, 44, 8);
    lv_obj_set_size(ui->shangxianshezhi_cont_1, 201, 85);
    lv_obj_set_scrollbar_mode(ui->shangxianshezhi_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for shangxianshezhi_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->shangxianshezhi_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->shangxianshezhi_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->shangxianshezhi_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->shangxianshezhi_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_cont_1, 24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->shangxianshezhi_cont_1, lv_color_hex(0x1191fa), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->shangxianshezhi_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->shangxianshezhi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->shangxianshezhi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->shangxianshezhi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->shangxianshezhi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_label_2
    ui->shangxianshezhi_label_2 = lv_label_create(ui->shangxianshezhi);
    lv_obj_set_pos(ui->shangxianshezhi_label_2, 92, 60);
    lv_obj_set_size(ui->shangxianshezhi_label_2, 95, 29);
    lv_label_set_text(ui->shangxianshezhi_label_2, "00.00");
    lv_label_set_long_mode(ui->shangxianshezhi_label_2, LV_LABEL_LONG_WRAP);

    //Write style for shangxianshezhi_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->shangxianshezhi_label_2, lv_color_hex(0xfb6501), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->shangxianshezhi_label_2, &lv_font_SourceHanSerifSC_Regular_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->shangxianshezhi_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->shangxianshezhi_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->shangxianshezhi_label_2, lv_color_hex(0x00ffcb), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->shangxianshezhi_label_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_label_3
    ui->shangxianshezhi_label_3 = lv_label_create(ui->shangxianshezhi);
    lv_obj_set_pos(ui->shangxianshezhi_label_3, 73, 17);
    lv_obj_set_size(ui->shangxianshezhi_label_3, 140, 43);
    lv_label_set_text(ui->shangxianshezhi_label_3, "输入电压上限");
    lv_label_set_long_mode(ui->shangxianshezhi_label_3, LV_LABEL_LONG_WRAP);

    //Write style for shangxianshezhi_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->shangxianshezhi_label_3, lv_color_hex(0x11fa5f), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->shangxianshezhi_label_3, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->shangxianshezhi_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->shangxianshezhi_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_cont_2
    ui->shangxianshezhi_cont_2 = lv_obj_create(ui->shangxianshezhi);
    lv_obj_set_pos(ui->shangxianshezhi_cont_2, 44, 102);
    lv_obj_set_size(ui->shangxianshezhi_cont_2, 201, 85);
    lv_obj_set_scrollbar_mode(ui->shangxianshezhi_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for shangxianshezhi_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->shangxianshezhi_cont_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->shangxianshezhi_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->shangxianshezhi_cont_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->shangxianshezhi_cont_2, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_cont_2, 24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->shangxianshezhi_cont_2, lv_color_hex(0x1191fa), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->shangxianshezhi_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->shangxianshezhi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->shangxianshezhi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->shangxianshezhi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->shangxianshezhi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_label_4
    ui->shangxianshezhi_label_4 = lv_label_create(ui->shangxianshezhi_cont_2);
    lv_obj_set_pos(ui->shangxianshezhi_label_4, 32, 2);
    lv_obj_set_size(ui->shangxianshezhi_label_4, 140, 43);
    lv_label_set_text(ui->shangxianshezhi_label_4, "输入电流上限");
    lv_label_set_long_mode(ui->shangxianshezhi_label_4, LV_LABEL_LONG_WRAP);

    //Write style for shangxianshezhi_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->shangxianshezhi_label_4, lv_color_hex(0xf1fd00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->shangxianshezhi_label_4, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->shangxianshezhi_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->shangxianshezhi_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_label_6
    ui->shangxianshezhi_label_6 = lv_label_create(ui->shangxianshezhi_cont_2);
    lv_obj_set_pos(ui->shangxianshezhi_label_6, 46, 49);
    lv_obj_set_size(ui->shangxianshezhi_label_6, 91, 29);
    lv_label_set_text(ui->shangxianshezhi_label_6, "00.00");
    lv_label_set_long_mode(ui->shangxianshezhi_label_6, LV_LABEL_LONG_WRAP);

    //Write style for shangxianshezhi_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->shangxianshezhi_label_6, lv_color_hex(0xfb6501), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->shangxianshezhi_label_6, &lv_font_SourceHanSerifSC_Regular_24, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->shangxianshezhi_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->shangxianshezhi_label_6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->shangxianshezhi_label_6, lv_color_hex(0x00ffcb), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->shangxianshezhi_label_6, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_label_5
    ui->shangxianshezhi_label_5 = lv_label_create(ui->shangxianshezhi);
    lv_obj_set_pos(ui->shangxianshezhi_label_5, 181, 70);
    lv_obj_set_size(ui->shangxianshezhi_label_5, 31, 22);
    lv_label_set_text(ui->shangxianshezhi_label_5, "v");
    lv_label_set_long_mode(ui->shangxianshezhi_label_5, LV_LABEL_LONG_WRAP);

    //Write style for shangxianshezhi_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->shangxianshezhi_label_5, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->shangxianshezhi_label_5, &lv_font_montserratMedium_17, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->shangxianshezhi_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->shangxianshezhi_label_5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_label_7
    ui->shangxianshezhi_label_7 = lv_label_create(ui->shangxianshezhi);
    lv_obj_set_pos(ui->shangxianshezhi_label_7, 177, 164);
    lv_obj_set_size(ui->shangxianshezhi_label_7, 31, 22);
    lv_label_set_text(ui->shangxianshezhi_label_7, "A");
    lv_label_set_long_mode(ui->shangxianshezhi_label_7, LV_LABEL_LONG_WRAP);

    //Write style for shangxianshezhi_label_7, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->shangxianshezhi_label_7, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->shangxianshezhi_label_7, &lv_font_montserratMedium_17, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->shangxianshezhi_label_7, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->shangxianshezhi_label_7, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_label_7, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes shangxianshezhi_btn_1
    ui->shangxianshezhi_btn_1 = lv_button_create(ui->shangxianshezhi);
    lv_obj_set_pos(ui->shangxianshezhi_btn_1, 89, 196);
    lv_obj_set_size(ui->shangxianshezhi_btn_1, 113, 40);
    ui->shangxianshezhi_btn_1_label = lv_label_create(ui->shangxianshezhi_btn_1);
    lv_label_set_text(ui->shangxianshezhi_btn_1_label, "确认返回");
    lv_label_set_long_mode(ui->shangxianshezhi_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->shangxianshezhi_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->shangxianshezhi_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->shangxianshezhi_btn_1_label, LV_PCT(100));

    //Write style for shangxianshezhi_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->shangxianshezhi_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->shangxianshezhi_btn_1, lv_color_hex(0xab00fd), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->shangxianshezhi_btn_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->shangxianshezhi_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->shangxianshezhi_btn_1, 12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->shangxianshezhi_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->shangxianshezhi_btn_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->shangxianshezhi_btn_1, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->shangxianshezhi_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->shangxianshezhi_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of shangxianshezhi.


    //Update current screen layout.
    lv_obj_update_layout(ui->shangxianshezhi);

}
