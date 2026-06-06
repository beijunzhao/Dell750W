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



void setup_scr_dianliujiaozhun(lv_ui *ui)
{
    //Write codes dianliujiaozhun
    ui->dianliujiaozhun = lv_obj_create(NULL);
    lv_obj_set_size(ui->dianliujiaozhun, 296, 240);
    lv_obj_set_scrollbar_mode(ui->dianliujiaozhun, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianliujiaozhun, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_cont_1
    ui->dianliujiaozhun_cont_1 = lv_obj_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_cont_1, 28, 33);
    lv_obj_set_size(ui->dianliujiaozhun_cont_1, 240, 81);
    lv_obj_set_scrollbar_mode(ui->dianliujiaozhun_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianliujiaozhun_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->dianliujiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->dianliujiaozhun_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->dianliujiaozhun_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_cont_1, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_cont_1, lv_color_hex(0x262525), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_32
    ui->dianliujiaozhun_label_32 = lv_label_create(ui->dianliujiaozhun_cont_1);
    lv_obj_set_pos(ui->dianliujiaozhun_label_32, 3, 4);
    lv_obj_set_size(ui->dianliujiaozhun_label_32, 83, 27);
    lv_label_set_text(ui->dianliujiaozhun_label_32, "校准目标");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_32, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_32, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_32, lv_color_hex(0x6a6363), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_32, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_32, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_32, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_32, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_31
    ui->dianliujiaozhun_label_31 = lv_label_create(ui->dianliujiaozhun_cont_1);
    lv_obj_set_pos(ui->dianliujiaozhun_label_31, 5, 31);
    lv_obj_set_size(ui->dianliujiaozhun_label_31, 88, 22);
    lv_label_set_text(ui->dianliujiaozhun_label_31, "实时电流");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_31, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_31, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_31, lv_color_hex(0x6a6363), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_31, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_31, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_31, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_31, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_30
    ui->dianliujiaozhun_label_30 = lv_label_create(ui->dianliujiaozhun_cont_1);
    lv_obj_set_pos(ui->dianliujiaozhun_label_30, 4, 56);
    lv_obj_set_size(ui->dianliujiaozhun_label_30, 95, 23);
    lv_label_set_text(ui->dianliujiaozhun_label_30, "当前PWM");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_30, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_30, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_30, lv_color_hex(0x6a6363), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_30, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_30, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_30, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_30, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_29
    ui->dianliujiaozhun_label_29 = lv_label_create(ui->dianliujiaozhun_cont_1);
    lv_obj_set_pos(ui->dianliujiaozhun_label_29, 116, 3);
    lv_obj_set_size(ui->dianliujiaozhun_label_29, 110, 23);
    lv_label_set_text(ui->dianliujiaozhun_label_29, "00.00 A");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_29, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_29, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_29, lv_color_hex(0xffd400), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_29, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_29, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_29, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_29, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_28
    ui->dianliujiaozhun_label_28 = lv_label_create(ui->dianliujiaozhun_cont_1);
    lv_obj_set_pos(ui->dianliujiaozhun_label_28, 160, 33);
    lv_obj_set_size(ui->dianliujiaozhun_label_28, 66, 26);
    lv_label_set_text(ui->dianliujiaozhun_label_28, "00000");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_28, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_28, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_28, lv_color_hex(0xffd400), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_28, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_28, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_28, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_28, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_27
    ui->dianliujiaozhun_label_27 = lv_label_create(ui->dianliujiaozhun_cont_1);
    lv_obj_set_pos(ui->dianliujiaozhun_label_27, 159, 57);
    lv_obj_set_size(ui->dianliujiaozhun_label_27, 67, 24);
    lv_label_set_text(ui->dianliujiaozhun_label_27, "00000");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_27, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_27, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_27, lv_color_hex(0xffd400), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_27, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_27, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_27, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_27, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_4
    ui->dianliujiaozhun_label_4 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_4, 0, 0);
    lv_obj_set_size(ui->dianliujiaozhun_label_4, 297, 30);
    lv_label_set_text(ui->dianliujiaozhun_label_4, "电流参考校准（Step6/6）");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_4, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_4, 3, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->dianliujiaozhun_label_4, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->dianliujiaozhun_label_4, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_4, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_4, &lv_font_SourceHanSerifSC_Regular_19, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_label_4, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_label_4, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_cont_2
    ui->dianliujiaozhun_cont_2 = lv_obj_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_cont_2, 25, 120);
    lv_obj_set_size(ui->dianliujiaozhun_cont_2, 245, 35);
    lv_obj_set_scrollbar_mode(ui->dianliujiaozhun_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianliujiaozhun_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_cont_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->dianliujiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->dianliujiaozhun_cont_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->dianliujiaozhun_cont_2, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_cont_2, 15, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_cont_2, lv_color_hex(0x00fcff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_img_1
    ui->dianliujiaozhun_img_1 = lv_image_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_img_1, 37, 129);
    lv_obj_set_size(ui->dianliujiaozhun_img_1, 14, 14);
    lv_obj_add_flag(ui->dianliujiaozhun_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->dianliujiaozhun_img_1, &_tishi_RGB565A8_14x14);
    lv_image_set_pivot(ui->dianliujiaozhun_img_1, 50,50);
    lv_image_set_rotation(ui->dianliujiaozhun_img_1, 0);

    //Write style for dianliujiaozhun_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->dianliujiaozhun_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->dianliujiaozhun_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_spangroup_1
    ui->dianliujiaozhun_spangroup_1 = lv_spangroup_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_spangroup_1, 55, 125);
    lv_obj_set_size(ui->dianliujiaozhun_spangroup_1, 215, 29);
    lv_spangroup_set_align(ui->dianliujiaozhun_spangroup_1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->dianliujiaozhun_spangroup_1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->dianliujiaozhun_spangroup_1, LV_SPAN_MODE_BREAK);
    //create span
    ui->dianliujiaozhun_spangroup_1_span = lv_spangroup_new_span(ui->dianliujiaozhun_spangroup_1);
    lv_span_set_text(ui->dianliujiaozhun_spangroup_1_span, "用万用表测量输出端电流，按上下键调节PWM 使电压等于目标值");
    lv_style_set_text_color(lv_span_get_style(ui->dianliujiaozhun_spangroup_1_span), lv_color_hex(0x005cff));
    lv_style_set_text_decor(lv_span_get_style(ui->dianliujiaozhun_spangroup_1_span), LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(lv_span_get_style(ui->dianliujiaozhun_spangroup_1_span), &lv_font_SourceHanSerifSC_Regular_14);

    //Write style state: LV_STATE_DEFAULT for &style_dianliujiaozhun_spangroup_1_main_main_default
    static lv_style_t style_dianliujiaozhun_spangroup_1_main_main_default;
    ui_init_style(&style_dianliujiaozhun_spangroup_1_main_main_default);

    lv_style_set_border_width(&style_dianliujiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_radius(&style_dianliujiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_bg_opa(&style_dianliujiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_pad_top(&style_dianliujiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_pad_right(&style_dianliujiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_dianliujiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_pad_left(&style_dianliujiaozhun_spangroup_1_main_main_default, 0);
    lv_style_set_shadow_width(&style_dianliujiaozhun_spangroup_1_main_main_default, 0);
    lv_obj_add_style(ui->dianliujiaozhun_spangroup_1, &style_dianliujiaozhun_spangroup_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->dianliujiaozhun_spangroup_1);

    //Write codes dianliujiaozhun_label_14
    ui->dianliujiaozhun_label_14 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_14, 87, 155);
    lv_obj_set_size(ui->dianliujiaozhun_label_14, 113, 21);
    lv_label_set_text(ui->dianliujiaozhun_label_14, "校准进度");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_14, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_14, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_14, lv_color_hex(0x7d7373), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_14, &lv_font_SourceHanSerifSC_Regular_21, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_14, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_14, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_15
    ui->dianliujiaozhun_label_15 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_15, 30, 179);
    lv_obj_set_size(ui->dianliujiaozhun_label_15, 25, 25);
    lv_label_set_text(ui->dianliujiaozhun_label_15, "1");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_15, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_15, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_15, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_15, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_15, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_15, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_15, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_15, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_label_15, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_label_15, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_16
    ui->dianliujiaozhun_label_16 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_16, 70, 179);
    lv_obj_set_size(ui->dianliujiaozhun_label_16, 25, 25);
    lv_label_set_text(ui->dianliujiaozhun_label_16, "2");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_16, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_16, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_16, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_16, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_16, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_16, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_16, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_16, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_label_16, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_label_16, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_17
    ui->dianliujiaozhun_label_17 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_17, 108, 179);
    lv_obj_set_size(ui->dianliujiaozhun_label_17, 25, 25);
    lv_label_set_text(ui->dianliujiaozhun_label_17, "3");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_17, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_17, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_17, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_17, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_17, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_17, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_17, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_17, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_label_17, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_label_17, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_17, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_18
    ui->dianliujiaozhun_label_18 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_18, 150, 179);
    lv_obj_set_size(ui->dianliujiaozhun_label_18, 25, 25);
    lv_label_set_text(ui->dianliujiaozhun_label_18, "4");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_18, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_18, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_18, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_18, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_18, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_18, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_18, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_18, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_label_18, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_label_18, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_19
    ui->dianliujiaozhun_label_19 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_19, 193, 179);
    lv_obj_set_size(ui->dianliujiaozhun_label_19, 25, 25);
    lv_label_set_text(ui->dianliujiaozhun_label_19, "5");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_19, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_19, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_19, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_19, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_19, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_19, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_19, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_19, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_label_19, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_label_19, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_20
    ui->dianliujiaozhun_label_20 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_20, 236, 179);
    lv_obj_set_size(ui->dianliujiaozhun_label_20, 25, 25);
    lv_label_set_text(ui->dianliujiaozhun_label_20, "6");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_20, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_20, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_20, 13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_20, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_20, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_20, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_20, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_20, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianliujiaozhun_label_20, lv_color_hex(0x0dff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianliujiaozhun_label_20, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_21
    ui->dianliujiaozhun_label_21 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_21, 24, 212);
    lv_obj_set_size(ui->dianliujiaozhun_label_21, 34, 21);
    lv_label_set_text(ui->dianliujiaozhun_label_21, "0.0A\n");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_21, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_21, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_21, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_21, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_21, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_21, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_22
    ui->dianliujiaozhun_label_22 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_22, 64, 212);
    lv_obj_set_size(ui->dianliujiaozhun_label_22, 34, 21);
    lv_label_set_text(ui->dianliujiaozhun_label_22, "5.0A");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_22, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_22, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_22, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_22, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_22, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_22, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_23
    ui->dianliujiaozhun_label_23 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_23, 102, 212);
    lv_obj_set_size(ui->dianliujiaozhun_label_23, 34, 21);
    lv_label_set_text(ui->dianliujiaozhun_label_23, "15A\n");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_23, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_23, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_23, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_23, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_23, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_23, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_24
    ui->dianliujiaozhun_label_24 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_24, 147, 212);
    lv_obj_set_size(ui->dianliujiaozhun_label_24, 34, 21);
    lv_label_set_text(ui->dianliujiaozhun_label_24, "30A\n");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_24, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_24, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_24, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_24, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_24, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_24, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_25
    ui->dianliujiaozhun_label_25 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_25, 189, 212);
    lv_obj_set_size(ui->dianliujiaozhun_label_25, 34, 21);
    lv_label_set_text(ui->dianliujiaozhun_label_25, "45A");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_25, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_25, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_25, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_25, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_25, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_25, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianliujiaozhun_label_26
    ui->dianliujiaozhun_label_26 = lv_label_create(ui->dianliujiaozhun);
    lv_obj_set_pos(ui->dianliujiaozhun_label_26, 232, 212);
    lv_obj_set_size(ui->dianliujiaozhun_label_26, 34, 21);
    lv_label_set_text(ui->dianliujiaozhun_label_26, "60A");
    lv_label_set_long_mode(ui->dianliujiaozhun_label_26, LV_LABEL_LONG_WRAP);

    //Write style for dianliujiaozhun_label_26, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianliujiaozhun_label_26, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianliujiaozhun_label_26, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianliujiaozhun_label_26, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianliujiaozhun_label_26, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianliujiaozhun_label_26, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of dianliujiaozhun.


    //Update current screen layout.
    lv_obj_update_layout(ui->dianliujiaozhun);

}
