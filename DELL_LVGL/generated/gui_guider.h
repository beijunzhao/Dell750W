/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *shouye;
	bool shouye_del;
	lv_obj_t *shouye_label_23;
	lv_obj_t *shouye_label_28;
	lv_obj_t *shouye_cont_1;
	lv_obj_t *shouye_label_30;
	lv_obj_t *shouye_label_31;
	lv_obj_t *shouye_label_33;
	lv_obj_t *shouye_label_34;
	lv_obj_t *shouye_label_48;
	lv_obj_t *shouye_label_49;
	lv_obj_t *shouye_btn_1;
	lv_obj_t *shouye_btn_1_label;
	lv_obj_t *shouye_cont_2;
	lv_obj_t *shouye_label_40;
	lv_obj_t *shouye_label_45;
	lv_obj_t *shouye_label_46;
	lv_obj_t *shouye_label_47;
	lv_obj_t *shouye_label_50;
	lv_obj_t *shouye_label_51;
	lv_obj_t *shouye_label_35;
	lv_obj_t *shouye_img_1;
	lv_obj_t *shouye_img_2;
	lv_obj_t *shouye_label_42;
	lv_obj_t *shouye_label_44;
	lv_obj_t *shouye_label_52;
	lv_obj_t *shouye_label_53;
	lv_obj_t *shouye_label_54;
	lv_obj_t *shouye_label_55;
	lv_obj_t *shouye_label_56;
	lv_obj_t *caidan;
	bool caidan_del;
	lv_obj_t *caidan_label_1;
	lv_obj_t *caidan_label_2;
	lv_obj_t *caidan_label_4;
	lv_obj_t *caidan_label_5;
	lv_obj_t *caidan_label_6;
	lv_obj_t *dianyajiaozhun;
	bool dianyajiaozhun_del;
	lv_obj_t *dianyajiaozhun_cont_1;
	lv_obj_t *dianyajiaozhun_label_8;
	lv_obj_t *dianyajiaozhun_label_9;
	lv_obj_t *dianyajiaozhun_label_10;
	lv_obj_t *dianyajiaozhun_label_11;
	lv_obj_t *dianyajiaozhun_label_12;
	lv_obj_t *dianyajiaozhun_label_13;
	lv_obj_t *dianyajiaozhun_label_4;
	lv_obj_t *dianyajiaozhun_cont_2;
	lv_obj_t *dianyajiaozhun_img_1;
	lv_obj_t *dianyajiaozhun_spangroup_1;
	lv_span_t *dianyajiaozhun_spangroup_1_span;
	lv_obj_t *dianyajiaozhun_label_14;
	lv_obj_t *dianyajiaozhun_label_15;
	lv_obj_t *dianyajiaozhun_label_16;
	lv_obj_t *dianyajiaozhun_label_17;
	lv_obj_t *dianyajiaozhun_label_18;
	lv_obj_t *dianyajiaozhun_label_19;
	lv_obj_t *dianyajiaozhun_label_20;
	lv_obj_t *dianyajiaozhun_label_21;
	lv_obj_t *dianyajiaozhun_label_22;
	lv_obj_t *dianyajiaozhun_label_23;
	lv_obj_t *dianyajiaozhun_label_24;
	lv_obj_t *dianyajiaozhun_label_25;
	lv_obj_t *dianyajiaozhun_label_26;
	lv_obj_t *dianliujiaozhun;
	bool dianliujiaozhun_del;
	lv_obj_t *dianliujiaozhun_cont_1;
	lv_obj_t *dianliujiaozhun_label_32;
	lv_obj_t *dianliujiaozhun_label_31;
	lv_obj_t *dianliujiaozhun_label_30;
	lv_obj_t *dianliujiaozhun_label_29;
	lv_obj_t *dianliujiaozhun_label_28;
	lv_obj_t *dianliujiaozhun_label_27;
	lv_obj_t *dianliujiaozhun_label_4;
	lv_obj_t *dianliujiaozhun_cont_2;
	lv_obj_t *dianliujiaozhun_img_1;
	lv_obj_t *dianliujiaozhun_spangroup_1;
	lv_span_t *dianliujiaozhun_spangroup_1_span;
	lv_obj_t *dianliujiaozhun_label_14;
	lv_obj_t *dianliujiaozhun_label_15;
	lv_obj_t *dianliujiaozhun_label_16;
	lv_obj_t *dianliujiaozhun_label_17;
	lv_obj_t *dianliujiaozhun_label_18;
	lv_obj_t *dianliujiaozhun_label_19;
	lv_obj_t *dianliujiaozhun_label_20;
	lv_obj_t *dianliujiaozhun_label_21;
	lv_obj_t *dianliujiaozhun_label_22;
	lv_obj_t *dianliujiaozhun_label_23;
	lv_obj_t *dianliujiaozhun_label_24;
	lv_obj_t *dianliujiaozhun_label_25;
	lv_obj_t *dianliujiaozhun_label_26;
	lv_obj_t *shangxianshezhi;
	bool shangxianshezhi_del;
	lv_obj_t *shangxianshezhi_cont_1;
	lv_obj_t *shangxianshezhi_label_2;
	lv_obj_t *shangxianshezhi_label_3;
	lv_obj_t *shangxianshezhi_cont_2;
	lv_obj_t *shangxianshezhi_label_4;
	lv_obj_t *shangxianshezhi_label_6;
	lv_obj_t *shangxianshezhi_label_5;
	lv_obj_t *shangxianshezhi_label_7;
	lv_obj_t *shangxianshezhi_btn_1;
	lv_obj_t *shangxianshezhi_btn_1_label;
	lv_obj_t *dianyuanxinxi;
	bool dianyuanxinxi_del;
	lv_obj_t *dianyuanxinxi_cont_1;
	lv_obj_t *dianyuanxinxi_label_3;
	lv_obj_t *dianyuanxinxi_label_4;
	lv_obj_t *dianyuanxinxi_label_5;
	lv_obj_t *dianyuanxinxi_label_6;
	lv_obj_t *dianyuanxinxi_label_21;
	lv_obj_t *dianyuanxinxi_label_22;
	lv_obj_t *dianyuanxinxi_label_23;
	lv_obj_t *dianyuanxinxi_label_24;
	lv_obj_t *dianyuanxinxi_label_25;
	lv_obj_t *dianyuanxinxi_label_1;
	lv_obj_t *dianyuanxinxi_label_2;
	lv_obj_t *dianyuanxinxi_label_12;
	lv_obj_t *dianyuanxinxi_cont_2;
	lv_obj_t *dianyuanxinxi_label_20;
	lv_obj_t *dianyuanxinxi_label_19;
	lv_obj_t *dianyuanxinxi_label_18;
	lv_obj_t *dianyuanxinxi_label_16;
	lv_obj_t *dianyuanxinxi_label_15;
	lv_obj_t *dianyuanxinxi_label_14;
	lv_obj_t *dianyuanxinxi_btn_1;
	lv_obj_t *dianyuanxinxi_btn_1_label;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_shouye(lv_ui *ui);
void setup_scr_caidan(lv_ui *ui);
void setup_scr_dianyajiaozhun(lv_ui *ui);
void setup_scr_dianliujiaozhun(lv_ui *ui);
void setup_scr_shangxianshezhi(lv_ui *ui);
void setup_scr_dianyuanxinxi(lv_ui *ui);
LV_IMAGE_DECLARE(_fensghan_RGB565A8_24x23);
LV_IMAGE_DECLARE(_wendyu_RGB565A8_19x21);
LV_IMAGE_DECLARE(_tishi_RGB565A8_14x14);

LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_16)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_33)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_36)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_13)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_22)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_15)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_17)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_19)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_25)
LV_FONT_DECLARE(lv_font_montserratMedium_23)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_24)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_23)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_20)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_18)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_14)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_21)
LV_FONT_DECLARE(lv_font_montserratMedium_17)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_12)


#ifdef __cplusplus
}
#endif
#endif
