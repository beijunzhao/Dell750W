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



void setup_scr_dianyuanxinxi(lv_ui *ui)
{
    //Write codes dianyuanxinxi
    ui->dianyuanxinxi = lv_obj_create(NULL);
    lv_obj_set_size(ui->dianyuanxinxi, 296, 240);
    lv_obj_set_scrollbar_mode(ui->dianyuanxinxi, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianyuanxinxi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyuanxinxi, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyuanxinxi, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_cont_1
    ui->dianyuanxinxi_cont_1 = lv_obj_create(ui->dianyuanxinxi);
    lv_obj_set_pos(ui->dianyuanxinxi_cont_1, 28, 28);
    lv_obj_set_size(ui->dianyuanxinxi_cont_1, 234, 97);
    lv_obj_set_scrollbar_mode(ui->dianyuanxinxi_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianyuanxinxi_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->dianyuanxinxi_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->dianyuanxinxi_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->dianyuanxinxi_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_cont_1, 17, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyuanxinxi_cont_1, lv_color_hex(0x658ad4), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyuanxinxi_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_3
    ui->dianyuanxinxi_label_3 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_3, 5, 21);
    lv_obj_set_size(ui->dianyuanxinxi_label_3, 56, 12);
    lv_label_set_text(ui->dianyuanxinxi_label_3, "型号");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_3, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_3, lv_color_hex(0xdadada), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_3, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_3, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_3, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_4
    ui->dianyuanxinxi_label_4 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_4, 4, 39);
    lv_obj_set_size(ui->dianyuanxinxi_label_4, 56, 13);
    lv_label_set_text(ui->dianyuanxinxi_label_4, "版本");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_4, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_4, lv_color_hex(0xdadada), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_4, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_4, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_4, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_5
    ui->dianyuanxinxi_label_5 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_5, 5, 57);
    lv_obj_set_size(ui->dianyuanxinxi_label_5, 57, 12);
    lv_label_set_text(ui->dianyuanxinxi_label_5, "产地");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_5, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_5, lv_color_hex(0xdadada), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_5, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_5, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_5, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_6
    ui->dianyuanxinxi_label_6 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_6, 5, 75);
    lv_obj_set_size(ui->dianyuanxinxi_label_6, 56, 14);
    lv_label_set_text(ui->dianyuanxinxi_label_6, "序列号");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_6, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_6, lv_color_hex(0xdadada), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_6, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_6, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_6, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_21
    ui->dianyuanxinxi_label_21 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_21, 86, 0);
    lv_obj_set_size(ui->dianyuanxinxi_label_21, 139, 14);
    lv_label_set_text(ui->dianyuanxinxi_label_21, "台达Delta\nASTEC / Emerson	雅达Astec");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_21, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_21, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_21, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_21, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_21, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_21, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_21, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_22
    ui->dianyuanxinxi_label_22 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_22, 166, 20);
    lv_obj_set_size(ui->dianyuanxinxi_label_22, 61, 12);
    lv_label_set_text(ui->dianyuanxinxi_label_22, "--");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_22, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_22, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_22, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_22, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_22, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_22, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_22, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_23
    ui->dianyuanxinxi_label_23 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_23, 203, 40);
    lv_obj_set_size(ui->dianyuanxinxi_label_23, 25, 11);
    lv_label_set_text(ui->dianyuanxinxi_label_23, "--");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_23, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_23, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_23, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_23, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_23, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_23, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_23, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_24
    ui->dianyuanxinxi_label_24 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_24, 179, 58);
    lv_obj_set_size(ui->dianyuanxinxi_label_24, 49, 12);
    lv_label_set_text(ui->dianyuanxinxi_label_24, "--");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_24, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_24, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_24, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_24, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_24, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_24, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_24, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_25
    ui->dianyuanxinxi_label_25 = lv_label_create(ui->dianyuanxinxi_cont_1);
    lv_obj_set_pos(ui->dianyuanxinxi_label_25, 85, 76);
    lv_obj_set_size(ui->dianyuanxinxi_label_25, 139, 12);
    lv_label_set_text(ui->dianyuanxinxi_label_25, "--");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_25, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_25, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_25, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_25, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_25, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_25, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_25, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_1
    ui->dianyuanxinxi_label_1 = lv_label_create(ui->dianyuanxinxi);
    lv_obj_set_pos(ui->dianyuanxinxi_label_1, 35, 33);
    lv_obj_set_size(ui->dianyuanxinxi_label_1, 56, 15);
    lv_label_set_text(ui->dianyuanxinxi_label_1, "厂商ID");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_1, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_1, lv_color_hex(0xdadada), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_1, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_2
    ui->dianyuanxinxi_label_2 = lv_label_create(ui->dianyuanxinxi);
    lv_obj_set_pos(ui->dianyuanxinxi_label_2, 85, 13);
    lv_obj_set_size(ui->dianyuanxinxi_label_2, 112, 15);
    lv_label_set_text(ui->dianyuanxinxi_label_2, "设备信息");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_2, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_2, &lv_font_SourceHanSerifSC_Regular_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_12
    ui->dianyuanxinxi_label_12 = lv_label_create(ui->dianyuanxinxi);
    lv_obj_set_pos(ui->dianyuanxinxi_label_12, 85, 130);
    lv_obj_set_size(ui->dianyuanxinxi_label_12, 112, 15);
    lv_label_set_text(ui->dianyuanxinxi_label_12, "电源规格");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_12, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_12, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_12, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_12, &lv_font_SourceHanSerifSC_Regular_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_12, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_12, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_12, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_cont_2
    ui->dianyuanxinxi_cont_2 = lv_obj_create(ui->dianyuanxinxi);
    lv_obj_set_pos(ui->dianyuanxinxi_cont_2, 31, 146);
    lv_obj_set_size(ui->dianyuanxinxi_cont_2, 231, 58);
    lv_obj_set_scrollbar_mode(ui->dianyuanxinxi_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for dianyuanxinxi_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_cont_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->dianyuanxinxi_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->dianyuanxinxi_cont_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->dianyuanxinxi_cont_2, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_cont_2, 17, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyuanxinxi_cont_2, lv_color_hex(0x658ad4), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyuanxinxi_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_20
    ui->dianyuanxinxi_label_20 = lv_label_create(ui->dianyuanxinxi_cont_2);
    lv_obj_set_pos(ui->dianyuanxinxi_label_20, 5, 4);
    lv_obj_set_size(ui->dianyuanxinxi_label_20, 97, 15);
    lv_label_set_text(ui->dianyuanxinxi_label_20, "最大输出电压");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_20, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_20, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_20, lv_color_hex(0xdadada), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_20, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_20, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_20, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_20, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_20, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_19
    ui->dianyuanxinxi_label_19 = lv_label_create(ui->dianyuanxinxi_cont_2);
    lv_obj_set_pos(ui->dianyuanxinxi_label_19, 4, 22);
    lv_obj_set_size(ui->dianyuanxinxi_label_19, 92, 14);
    lv_label_set_text(ui->dianyuanxinxi_label_19, "最大输出电流");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_19, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_19, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_19, lv_color_hex(0xdadada), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_19, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_19, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_19, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_19, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_19, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_18
    ui->dianyuanxinxi_label_18 = lv_label_create(ui->dianyuanxinxi_cont_2);
    lv_obj_set_pos(ui->dianyuanxinxi_label_18, 5, 40);
    lv_obj_set_size(ui->dianyuanxinxi_label_18, 96, 15);
    lv_label_set_text(ui->dianyuanxinxi_label_18, "最大输出功率");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_18, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_18, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_18, lv_color_hex(0xdadada), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_18, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_18, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_18, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_18, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_18, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_16
    ui->dianyuanxinxi_label_16 = lv_label_create(ui->dianyuanxinxi_cont_2);
    lv_obj_set_pos(ui->dianyuanxinxi_label_16, 142, 4);
    lv_obj_set_size(ui->dianyuanxinxi_label_16, 81, 13);
    lv_label_set_text(ui->dianyuanxinxi_label_16, "12.0V");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_16, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_16, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_16, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_16, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_16, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_16, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_16, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_15
    ui->dianyuanxinxi_label_15 = lv_label_create(ui->dianyuanxinxi_cont_2);
    lv_obj_set_pos(ui->dianyuanxinxi_label_15, 140, 21);
    lv_obj_set_size(ui->dianyuanxinxi_label_15, 83, 13);
    lv_label_set_text(ui->dianyuanxinxi_label_15, "62.5A");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_15, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_15, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_15, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_15, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_15, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_15, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_15, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_label_14
    ui->dianyuanxinxi_label_14 = lv_label_create(ui->dianyuanxinxi_cont_2);
    lv_obj_set_pos(ui->dianyuanxinxi_label_14, 142, 41);
    lv_obj_set_size(ui->dianyuanxinxi_label_14, 81, 14);
    lv_label_set_text(ui->dianyuanxinxi_label_14, "750W");
    lv_label_set_long_mode(ui->dianyuanxinxi_label_14, LV_LABEL_LONG_WRAP);

    //Write style for dianyuanxinxi_label_14, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_label_14, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_label_14, &lv_font_SourceHanSerifSC_Regular_13, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_label_14, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_label_14, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_label_14, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes dianyuanxinxi_btn_1
    ui->dianyuanxinxi_btn_1 = lv_button_create(ui->dianyuanxinxi);
    lv_obj_set_pos(ui->dianyuanxinxi_btn_1, 91, 210);
    lv_obj_set_size(ui->dianyuanxinxi_btn_1, 110, 23);
    ui->dianyuanxinxi_btn_1_label = lv_label_create(ui->dianyuanxinxi_btn_1);
    lv_label_set_text(ui->dianyuanxinxi_btn_1_label, "返回");
    lv_label_set_long_mode(ui->dianyuanxinxi_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->dianyuanxinxi_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->dianyuanxinxi_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->dianyuanxinxi_btn_1_label, LV_PCT(100));

    //Write style for dianyuanxinxi_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->dianyuanxinxi_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->dianyuanxinxi_btn_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->dianyuanxinxi_btn_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->dianyuanxinxi_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->dianyuanxinxi_btn_1, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->dianyuanxinxi_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->dianyuanxinxi_btn_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->dianyuanxinxi_btn_1, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->dianyuanxinxi_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->dianyuanxinxi_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of dianyuanxinxi.


    //Update current screen layout.
    lv_obj_update_layout(ui->dianyuanxinxi);

}
