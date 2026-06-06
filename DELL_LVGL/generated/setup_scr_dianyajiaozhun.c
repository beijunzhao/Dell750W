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



void setup_scr_dianyajiaozhun(lv_ui *ui)
{
    //Write codes dianyajiaozhun
    ui->dianyajiaozhun = lv_obj_create(NULL);
    lv_obj_set_size(ui->dianyajiaozhun, 296, 240);
    lv_obj_set_scrollbar_mode(ui->dianyajiaozhun, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianyajiaozhun, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_cont_1
    ui->dianyajiaozhun_cont_1 = lv_obj_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_cont_1, 28, 33);
    lv_obj_set_size(ui->dianyajiaozhun_cont_1, 240, 81);
    lv_obj_set_scrollbar_mode(ui->dianyajiaozhun_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianyajiaozhun_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->dianyajiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->dianyajiaozhun_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->dianyajiaozhun_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_cont_1, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_cont_1, lv_color_hex(0x262525), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_8
    ui->dianyajiaozhun_label_8 = lv_label_create(ui->dianyajiaozhun_cont_1);
    lv_obj_set_pos(ui->dianyajiaozhun_label_8, 1, 6);
    lv_obj_set_size(ui->dianyajiaozhun_label_8, 83, 27);
    lv_label_set_text(ui->dianyajiaozhun_label_8, "校准目标");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_8, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_8, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_8, lv_color_hex(0x6a6363), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_8, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_8, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_8, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_8, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_9
    ui->dianyajiaozhun_label_9 = lv_label_create(ui->dianyajiaozhun_cont_1);
    lv_obj_set_pos(ui->dianyajiaozhun_label_9, 5, 31);
    lv_obj_set_size(ui->dianyajiaozhun_label_9, 88, 22);
    lv_label_set_text(ui->dianyajiaozhun_label_9, "实时ADC");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_9, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_9, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_9, lv_color_hex(0x6a6363), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_9, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_9, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_9, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_9, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_10
    ui->dianyajiaozhun_label_10 = lv_label_create(ui->dianyajiaozhun_cont_1);
    lv_obj_set_pos(ui->dianyajiaozhun_label_10, 4, 56);
    lv_obj_set_size(ui->dianyajiaozhun_label_10, 95, 23);
    lv_label_set_text(ui->dianyajiaozhun_label_10, "当前PWM");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_10, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_10, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_10, lv_color_hex(0x6a6363), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_10, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_10, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_10, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_10, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_11
    ui->dianyajiaozhun_label_11 = lv_label_create(ui->dianyajiaozhun_cont_1);
    lv_obj_set_pos(ui->dianyajiaozhun_label_11, 116, 3);
    lv_obj_set_size(ui->dianyajiaozhun_label_11, 110, 23);
    lv_label_set_text(ui->dianyajiaozhun_label_11, "00.00 V");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_11, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_11, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_11, lv_color_hex(0xffd400), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_11, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_11, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_11, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_11, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_12
    ui->dianyajiaozhun_label_12 = lv_label_create(ui->dianyajiaozhun_cont_1);
    lv_obj_set_pos(ui->dianyajiaozhun_label_12, 160, 33);
    lv_obj_set_size(ui->dianyajiaozhun_label_12, 66, 26);
    lv_label_set_text(ui->dianyajiaozhun_label_12, "00000");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_12, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_12, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_12, lv_color_hex(0xffd400), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_12, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_12, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_12, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_13
    ui->dianyajiaozhun_label_13 = lv_label_create(ui->dianyajiaozhun_cont_1);
    lv_obj_set_pos(ui->dianyajiaozhun_label_13, 159, 57);
    lv_obj_set_size(ui->dianyajiaozhun_label_13, 67, 24);
    lv_label_set_text(ui->dianyajiaozhun_label_13, "00000");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_13, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_13, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_13, lv_color_hex(0xffd400), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_13, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_13, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_13, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_13, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_4
    ui->dianyajiaozhun_label_4 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_4, 0, 0);
    lv_obj_set_size(ui->dianyajiaozhun_label_4, 297, 29);
    lv_label_set_text(ui->dianyajiaozhun_label_4, "电压参考校准（Step6/6）");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_4, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_4, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->dianyajiaozhun_label_4, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->dianyajiaozhun_label_4, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_4, &lv_font_SourceHanSerifSC_Regular_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_label_4, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_label_4, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_cont_2
    ui->dianyajiaozhun_cont_2 = lv_obj_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_cont_2, 25, 120);
    lv_obj_set_size(ui->dianyajiaozhun_cont_2, 245, 35);
    lv_obj_set_scrollbar_mode(ui->dianyajiaozhun_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianyajiaozhun_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_cont_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->dianyajiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->dianyajiaozhun_cont_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->dianyajiaozhun_cont_2, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_cont_2, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_cont_2, lv_color_hex(0x00fcff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_img_1
    ui->dianyajiaozhun_img_1 = lv_image_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_img_1, 37, 129);
    lv_obj_set_size(ui->dianyajiaozhun_img_1, 14, 14);
    lv_obj_add_flag(ui->dianyajiaozhun_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->dianyajiaozhun_img_1, &_tishi_RGB565A8_14x14);
    lv_image_set_pivot(ui->dianyajiaozhun_img_1, 50,50);
    lv_image_set_rotation(ui->dianyajiaozhun_img_1, 0);

    //Write style for dianyajiaozhun_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->dianyajiaozhun_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->dianyajiaozhun_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_spangroup_1
    ui->dianyajiaozhun_spangroup_1 = lv_spangroup_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_spangroup_1, 55, 125);
    lv_obj_set_size(ui->dianyajiaozhun_spangroup_1, 215, 29);
    lv_spangroup_set_align(ui->dianyajiaozhun_spangroup_1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->dianyajiaozhun_spangroup_1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->dianyajiaozhun_spangroup_1, LV_SPAN_MODE_BREAK);
    //create span
    ui->dianyajiaozhun_spangroup_1_span = lv_spangroup_new_span(ui->dianyajiaozhun_spangroup_1);
    lv_span_set_text(ui->dianyajiaozhun_spangroup_1_span, "用万用表测量输出端电压，按上下键调节PWM 使电压等于目标值");
    lv_style_set_text_color(lv_span_get_style(ui->dianyajiaozhun_spangroup_1_span), lv_color_hex(0x005cff));
    lv_style_set_text_decor(lv_span_get_style(ui->dianyajiaozhun_spangroup_1_span), LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(lv_span_get_style(ui->dianyajiaozhun_spangroup_1_span), &lv_font_SourceHanSerifSC_Regular_14);

    //Write style state: LV_STATE_DEFAULT for &style_dianyajiaozhun_spangroup_1_main_main_default
    static lv_style_t style_dianyajiaozhun_spangroup_1_main_main_default;
    ui_init_style(&style_dianyajiaozhun_spangroup_1_main_main_default);

    lv_style_set_border_width(&style_dianyajiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_radius(&style_dianyajiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_bg_opa(&style_dianyajiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_pad_top(&style_dianyajiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_pad_right(&style_dianyajiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_dianyajiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_pad_left(&style_dianyajiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_shadow_width(&style_dianyajiaozhun_spangroup_1_main_main_default, 0);
    lv_obj_add_style(ui->dianyajiaozhun_spangroup_1, &style_dianyajiaozhun_spangroup_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->dianyajiaozhun_spangroup_1);

    //Write codes dianyajiaozhun_label_14
    ui->dianyajiaozhun_label_14 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_14, 87, 155);
    lv_obj_set_size(ui->dianyajiaozhun_label_14, 113, 21);
    lv_label_set_text(ui->dianyajiaozhun_label_14, "校准进度");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_14, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_14, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_14, lv_color_hex(0x7d7373), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_14, &lv_font_SourceHanSerifSC_Regular_21, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_14, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_14, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_15
    ui->dianyajiaozhun_label_15 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_15, 30, 179);
    lv_obj_set_size(ui->dianyajiaozhun_label_15, 25, 25);
    lv_label_set_text(ui->dianyajiaozhun_label_15, "1");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_15, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_15, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_15, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_15, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_15, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_15, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_15, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_15, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_label_15, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_label_15, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_16
    ui->dianyajiaozhun_label_16 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_16, 70, 179);
    lv_obj_set_size(ui->dianyajiaozhun_label_16, 25, 25);
    lv_label_set_text(ui->dianyajiaozhun_label_16, "2");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_16, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_16, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_16, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_16, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_16, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_16, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_16, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_16, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_label_16, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_label_16, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_17
    ui->dianyajiaozhun_label_17 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_17, 108, 179);
    lv_obj_set_size(ui->dianyajiaozhun_label_17, 25, 25);
    lv_label_set_text(ui->dianyajiaozhun_label_17, "3");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_17, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_17, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_17, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_17, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_17, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_17, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_17, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_17, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_label_17, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_label_17, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_18
    ui->dianyajiaozhun_label_18 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_18, 150, 179);
    lv_obj_set_size(ui->dianyajiaozhun_label_18, 25, 25);
    lv_label_set_text(ui->dianyajiaozhun_label_18, "4");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_18, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_18, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_18, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_18, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_18, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_18, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_18, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_18, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_label_18, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_label_18, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_19
    ui->dianyajiaozhun_label_19 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_19, 193, 179);
    lv_obj_set_size(ui->dianyajiaozhun_label_19, 25, 25);
    lv_label_set_text(ui->dianyajiaozhun_label_19, "5");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_19, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_19, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_19, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_19, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_19, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_19, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_19, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_19, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_label_19, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_label_19, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_20
    ui->dianyajiaozhun_label_20 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_20, 236, 179);
    lv_obj_set_size(ui->dianyajiaozhun_label_20, 25, 25);
    lv_label_set_text(ui->dianyajiaozhun_label_20, "6");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_20, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_20, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_20, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_20, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_20, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_20, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_20, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_20, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyajiaozhun_label_20, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyajiaozhun_label_20, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_21
    ui->dianyajiaozhun_label_21 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_21, 24, 212);
    lv_obj_set_size(ui->dianyajiaozhun_label_21, 34, 21);
    lv_label_set_text(ui->dianyajiaozhun_label_21, "0.0V");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_21, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_21, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_21, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_21, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_21, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_21, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_22
    ui->dianyajiaozhun_label_22 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_22, 64, 211);
    lv_obj_set_size(ui->dianyajiaozhun_label_22, 34, 21);
    lv_label_set_text(ui->dianyajiaozhun_label_22, "1.0V");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_22, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_22, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_22, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_22, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_22, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_22, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_23
    ui->dianyajiaozhun_label_23 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_23, 102, 211);
    lv_obj_set_size(ui->dianyajiaozhun_label_23, 34, 21);
    lv_label_set_text(ui->dianyajiaozhun_label_23, "2.5V");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_23, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_23, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_23, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_23, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_23, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_23, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_24
    ui->dianyajiaozhun_label_24 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_24, 147, 211);
    lv_obj_set_size(ui->dianyajiaozhun_label_24, 34, 21);
    lv_label_set_text(ui->dianyajiaozhun_label_24, "5.0V");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_24, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_24, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_24, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_24, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_24, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_24, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_25
    ui->dianyajiaozhun_label_25 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_25, 189, 211);
    lv_obj_set_size(ui->dianyajiaozhun_label_25, 34, 21);
    lv_label_set_text(ui->dianyajiaozhun_label_25, "10V");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_25, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_25, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_25, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_25, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_25, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_25, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyajiaozhun_label_26
    ui->dianyajiaozhun_label_26 = lv_label_create(ui->dianyajiaozhun);
    lv_obj_set_pos(ui->dianyajiaozhun_label_26, 232, 211);
    lv_obj_set_size(ui->dianyajiaozhun_label_26, 34, 21);
    lv_label_set_text(ui->dianyajiaozhun_label_26, "12V");
    lv_label_set_long_mode(ui->dianyajiaozhun_label_26, LV_LABEL_LONG_WRAP);

    //Write style for dianyajiaozhun_label_26, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyajiaozhun_label_26, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyajiaozhun_label_26, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyajiaozhun_label_26, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyajiaozhun_label_26, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyajiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of dianyajiaozhun.


    //Update current screen layout.
    lv_obj_update_layout(ui->dianyajiaozhun);

}
