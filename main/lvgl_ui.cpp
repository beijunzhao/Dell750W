/**
 * lvgl_ui.cpp - 电源监控仪表盘 UI (LVGL v9)
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

static uint32_t s_run_sec=0,s_run_tick=0;
static inline uint32_t _now_sec(void){return(uint32_t)(esp_timer_get_time()/1000000ULL);}
static void _run_stop(void){if(s_run_tick>0){s_run_sec+=_now_sec()-s_run_tick;s_run_tick=0;}}
static void _run_start(void){s_run_tick=_now_sec();}
static uint32_t _run_total(void){uint32_t t=s_run_sec;if(s_run_tick>0)t+=_now_sec()-s_run_tick;return t;}

typedef enum{PAGE_MAIN=0,PAGE_MENU,PAGE_CALIB_V,PAGE_CALIB_I,PAGE_RANGE,PAGE_INFO}page_t;
static page_t s_page=PAGE_MAIN;

#define FOCUS_COUNT 3
typedef enum{FOCUS_POWER=0,FOCUS_VSET,FOCUS_ISET}focus_t;
static int s_focus=FOCUS_POWER;

/* PAGE_RANGE 焦点: 0=V上限, 1=I上限, 2=确认返回按钮 */
static int s_range_focus=0;
#define RANGE_FOCUS_COUNT 3
static bool s_info_loaded=false;

#define MENU_COUNT 5
static int s_menu_idx=0;

static uint32_t s_last_ok_ms=0;
#define DOUBLE_CLICK_MS 400
typedef enum{EDIT_NONE,EDIT_V,EDIT_I,EDIT_RANGE_V,EDIT_RANGE_I}edit_t;
static edit_t s_edit=EDIT_NONE;
static int s_epos=0;
static bool s_blink=true;

/* 编辑期间暂存的上限值 */
static float s_edit_vmax=0.0f;
static float s_edit_imax=0.0f;

static void _switch_to_page(page_t page);
static void _apply_focus(void),_clear_focus(void);
static void _apply_range_focus(void);
static void _apply_menu_cursor(void);
static void _enter_edit(edit_t mode),_exit_edit(void);
static void _ed_up(void),_ed_dn(void),_ed_ok(void),_ed_show(void);
static void _digits(float v,int d[4]);
static float _value(const int d[4]);
static const char* _parse_mfr_name(const char* id);

static lv_obj_t*s_menu_cursor=NULL;

void lvgl_ui_init(void){
    if(s_ui_ready)return;
    ESP_LOGI(TAG,"========== LVGL UI Init ==========");
    setup_ui(&guider_ui);
    setup_scr_caidan(&guider_ui);
    setup_scr_dianyajiaozhun(&guider_ui);
    setup_scr_dianliujiaozhun(&guider_ui);
    setup_scr_shangxianshezhi(&guider_ui);
    setup_scr_dianyuanxinxi(&guider_ui);

    s_menu_cursor=lv_label_create(guider_ui.caidan);
    lv_obj_set_pos(s_menu_cursor,30,64);
    lv_obj_set_size(s_menu_cursor,20,24);
    lv_label_set_text(s_menu_cursor,">");
    lv_obj_set_style_text_color(s_menu_cursor,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(s_menu_cursor,&lv_font_SourceHanSerifSC_Regular_24,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(s_menu_cursor,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);

    guider_ui.shouye_del=false;guider_ui.caidan_del=false;
    guider_ui.dianyajiaozhun_del=false;guider_ui.dianliujiaozhun_del=false;
    guider_ui.shangxianshezhi_del=false;guider_ui.dianyuanxinxi_del=false;
    s_page=PAGE_MAIN;lv_screen_load(guider_ui.shouye);

    s_update_timer=lv_timer_create([](lv_timer_t*t){
        (void)t;if(!s_ui_ready)return;
        lv_ui*u=&guider_ui;char b[24];
        if(s_page==PAGE_MAIN){
            if(s_edit!=EDIT_NONE)s_blink=!s_blink;
            {float v=ADCSampler::getVoltage();int ip=(int)v,fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;}snprintf(b,sizeof(b),"%02d.%02d",ip,fp);lv_label_set_text(u->shouye_label_49,b);}
            {float v=PMBus::I_out;int ip=(int)v,fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;}snprintf(b,sizeof(b),"%02d.%02d",ip,fp);lv_label_set_text(u->shouye_label_33,b);}
            {float v=PMBus::W_out;if(v>=100)snprintf(b,sizeof(b),"%.0f",v);else if(v>=10)snprintf(b,sizeof(b),"%.1f",v);else{int ip=(int)v,fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;}snprintf(b,sizeof(b),"%02d.%02d",ip,fp);}lv_label_set_text(u->shouye_label_34,b);}
            if(u->shouye_label_52){if(s_edit==EDIT_V)_ed_show();else{float v=PowerControl::getSetVoltage();int ip=(int)v,fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;}snprintf(b,sizeof(b),"%02d.%02d",ip,fp);lv_label_set_text(u->shouye_label_52,b);}}
            if(u->shouye_label_54){if(s_edit==EDIT_I)_ed_show();else{float v=PowerControl::getSetCurrent();int ip=(int)v,fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;}snprintf(b,sizeof(b),"%02d.%02d",ip,fp);lv_label_set_text(u->shouye_label_54,b);}}
            if(u->shouye_btn_1_label){if(PowerControl::isPoweredOn()){lv_label_set_text(u->shouye_btn_1_label,"ON");lv_obj_set_style_bg_color(u->shouye_btn_1,lv_color_hex(0x00E676),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}else{lv_label_set_text(u->shouye_btn_1_label,"OFF");lv_obj_set_style_bg_color(u->shouye_btn_1,lv_color_hex(0xFF1744),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}}
            if(u->shouye_label_46){float wh=PMBus::E_out,v=ADCSampler::getVoltage();float ah=(v>1)?(wh/v):0;snprintf(b,sizeof(b),"%06.2f",ah);lv_label_set_text(u->shouye_label_46,b);}
            if(u->shouye_label_45){float e=PMBus::E_out;snprintf(b,sizeof(b),"%06.2f",e);lv_label_set_text(u->shouye_label_45,b);}
            if(u->shouye_label_51){uint32_t t_=_run_total();snprintf(b,sizeof(b),"%02d:%02d:%02d",(int)(t_/3600),(int)((t_%3600)/60),(int)(t_%60));lv_label_set_text(u->shouye_label_51,b);}
            if(u->shouye_label_42){snprintf(b,sizeof(b),"%.0f",PMBus::fanSpeed[0]);lv_label_set_text(u->shouye_label_42,b);}
            if(u->shouye_label_44){snprintf(b,sizeof(b),"%.0f",PMBus::temperature[0]);lv_label_set_text(u->shouye_label_44,b);}
            if(u->shouye_label_55){snprintf(b,sizeof(b),"%.0f",PMBus::temperature[1]);lv_label_set_text(u->shouye_label_55,b);}
            if(u->shouye_label_56){snprintf(b,sizeof(b),"%.0f",PMBus::temperature[2]);lv_label_set_text(u->shouye_label_56,b);}
        }
        else if(s_page==PAGE_CALIB_V&&calibration_is_active()){
            int step=calibration_get_current_step();if(step<0)step=0;
            if(u->dianyajiaozhun_label_4){char big[36];snprintf(big,sizeof(big),"电压校准 Step%d/6",step+1);lv_label_set_text(u->dianyajiaozhun_label_4,big);}
            if(u->dianyajiaozhun_label_11){float tgt=g_calib_v_targets[step];snprintf(b,sizeof(b),"%.2f V",tgt);lv_label_set_text(u->dianyajiaozhun_label_11,b);}
            if(u->dianyajiaozhun_label_12){int adc=ADCSampler::getRawAdc();snprintf(b,sizeof(b),"%d",adc);lv_label_set_text(u->dianyajiaozhun_label_12,b);}
            if(u->dianyajiaozhun_label_13){int pwm=calibration_get_pwm();snprintf(b,sizeof(b),"%d",pwm);lv_label_set_text(u->dianyajiaozhun_label_13,b);}
            const calib_point_t*data=calibration_get_data();
            lv_obj_t*dots[6]={u->dianyajiaozhun_label_15,u->dianyajiaozhun_label_16,u->dianyajiaozhun_label_17,u->dianyajiaozhun_label_18,u->dianyajiaozhun_label_19,u->dianyajiaozhun_label_20};
            for(int i=0;i<6;i++)if(dots[i]){if(data[i].adc_raw!=0)lv_obj_set_style_bg_color(dots[i],lv_color_hex(0x0dff00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);else if(i==step)lv_obj_set_style_bg_color(dots[i],lv_color_hex(0xFF9100),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);else lv_obj_set_style_bg_color(dots[i],lv_color_hex(0x555555),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
        }
        else if(s_page==PAGE_RANGE){
            if(s_edit!=EDIT_RANGE_V&&s_edit!=EDIT_RANGE_I){
                /* 非编辑: 显示当前上限值 */
                if(u->shangxianshezhi_label_2){float v=PowerControl::getVMax();int ip=(int)v,fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;}snprintf(b,sizeof(b),"%02d.%02d",ip,fp);lv_label_set_text(u->shangxianshezhi_label_2,b);}
                if(u->shangxianshezhi_label_6){float v=PowerControl::getIMax();int ip=(int)v,fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;}snprintf(b,sizeof(b),"%02d.%02d",ip,fp);lv_label_set_text(u->shangxianshezhi_label_6,b);}
            }else{
                s_blink=!s_blink;
                /* 编辑中: 闪烁显示 */
                if(s_edit==EDIT_RANGE_V&&u->shangxianshezhi_label_2){_ed_show();}
                if(s_edit==EDIT_RANGE_I&&u->shangxianshezhi_label_6){_ed_show();}
            }
        }
        else if(s_page==PAGE_CALIB_I&&calibration_is_active()){
            int step=calibration_get_current_step();if(step<0)step=0;
            if(u->dianliujiaozhun_label_4){char big[36];snprintf(big,sizeof(big),"电流校准 Step%d/6",step+1);lv_label_set_text(u->dianliujiaozhun_label_4,big);}
            if(u->dianliujiaozhun_label_29){float tgt=g_calib_i_targets[step];snprintf(b,sizeof(b),"%.1f A",tgt);lv_label_set_text(u->dianliujiaozhun_label_29,b);}
            if(u->dianliujiaozhun_label_28){float cur=calibration_get_pmbus_value();snprintf(b,sizeof(b),"%.2f",cur);lv_label_set_text(u->dianliujiaozhun_label_28,b);}
            if(u->dianliujiaozhun_label_27){int pwm=calibration_get_pwm();snprintf(b,sizeof(b),"%d",pwm);lv_label_set_text(u->dianliujiaozhun_label_27,b);}
            lv_obj_t*idots[6]={u->dianliujiaozhun_label_15,u->dianliujiaozhun_label_16,u->dianliujiaozhun_label_17,u->dianliujiaozhun_label_18,u->dianliujiaozhun_label_19,u->dianliujiaozhun_label_20};
            const calib_point_t*idata=calibration_get_data();
            for(int i=0;i<6;i++)if(idots[i]){if(idata[i].pmbus_i_raw>0.0f)lv_obj_set_style_bg_color(idots[i],lv_color_hex(0x0dff00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);else if(i==step)lv_obj_set_style_bg_color(idots[i],lv_color_hex(0xFF9100),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);else lv_obj_set_style_bg_color(idots[i],lv_color_hex(0x555555),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
        }
        else if(s_page==PAGE_INFO&&!s_info_loaded){
            s_info_loaded=true;
            char bi[48];

            /* 只更新文本，不修改任何位置/宽度/对齐属性，完全保持 GUI Guider 原始布局 */
            auto set_text = [](lv_obj_t* lb, const char* txt){
                if(!lb) return;
                lv_label_set_text(lb, txt);
            };

            if(u->dianyuanxinxi_label_21){ set_text(u->dianyuanxinxi_label_21, _parse_mfr_name(PMBus::mfrId)); }

            if(u->dianyuanxinxi_label_22){
                snprintf(bi,sizeof(bi),"%s",PMBus::mfrModel[0]?PMBus::mfrModel:"N/A");
                set_text(u->dianyuanxinxi_label_22, bi);
            }
            if(u->dianyuanxinxi_label_23){
                snprintf(bi,sizeof(bi),"%s",PMBus::mfrRevision[0]?PMBus::mfrRevision:"N/A");
                set_text(u->dianyuanxinxi_label_23, bi);
            }
            if(u->dianyuanxinxi_label_24){
                snprintf(bi,sizeof(bi),"%s",PMBus::mfrLocation[0]?PMBus::mfrLocation:"N/A");
                set_text(u->dianyuanxinxi_label_24, bi);
            }
            if(u->dianyuanxinxi_label_25){
                snprintf(bi,sizeof(bi),"%s",PMBus::mfrSerial[0]?PMBus::mfrSerial:"N/A");
                set_text(u->dianyuanxinxi_label_25, bi);
            }

            if(u->dianyuanxinxi_label_16){
                snprintf(bi,sizeof(bi),"%.1fV",PowerControl::getVMax());
                set_text(u->dianyuanxinxi_label_16, bi);
            }
            if(u->dianyuanxinxi_label_15){
                snprintf(bi,sizeof(bi),"%.1fA",PowerControl::getIMax());
                set_text(u->dianyuanxinxi_label_15, bi);
            }
            if(u->dianyuanxinxi_label_14){
                snprintf(bi,sizeof(bi),"%.0fW",PowerControl::getVMax()*PowerControl::getIMax());
                set_text(u->dianyuanxinxi_label_14, bi);
            }
        }
    },500,NULL);
    lv_timer_ready(s_update_timer);

    s_focus=FOCUS_POWER;s_edit=EDIT_NONE;
    s_run_sec=0;s_run_tick=0;
    if(PowerControl::isPoweredOn())s_run_tick=_now_sec();
    _apply_focus();s_ui_ready=true;
    ESP_LOGI(TAG,"========== LVGL UI Ready ==========");
}

void lvgl_ui_deinit(void){if(!s_ui_ready)return;if(s_update_timer){lv_timer_delete(s_update_timer);s_update_timer=NULL;}s_ui_ready=false;}
bool lvgl_ui_is_ready(void){return s_ui_ready;}

void lvgl_show_calibration(void){lv_screen_load(guider_ui.dianyajiaozhun);s_page=PAGE_CALIB_V;}
void lvgl_show_current_calibration(void){lv_screen_load(guider_ui.dianliujiaozhun);s_page=PAGE_CALIB_I;}
void lvgl_hide_calibration(void){_switch_to_page(PAGE_MENU);}
void lvgl_reset_debounce(void);

static void _switch_to_page(page_t page){
    lvgl_reset_debounce();
    if(page==PAGE_MAIN){lv_screen_load(guider_ui.shouye);s_page=PAGE_MAIN;_apply_focus();}
    else if(page==PAGE_MENU){lv_screen_load(guider_ui.caidan);s_page=PAGE_MENU;s_menu_idx=0;_apply_menu_cursor();}
    else if(page==PAGE_RANGE){lv_screen_load(guider_ui.shangxianshezhi);s_page=PAGE_RANGE;s_range_focus=0;_apply_range_focus();}
    else if(page==PAGE_INFO){lv_screen_load(guider_ui.dianyuanxinxi);s_page=PAGE_INFO;s_info_loaded=false;}
}

void lvgl_ui_handle_key(lvgl_key_t key){
    if(!s_ui_ready)return;

    if(s_page==PAGE_CALIB_V){
        if(key==LVGL_KEY_OK_LONG){if(calibration_is_active())calibration_stop();_switch_to_page(PAGE_MENU);}
        else if(calibration_is_active())calibration_handle_button(key==LVGL_KEY_UP,key==LVGL_KEY_DOWN,key==LVGL_KEY_OK);
        return;
    }
    if(s_page==PAGE_CALIB_I){
        if(key==LVGL_KEY_OK_LONG){if(calibration_is_active())calibration_stop();_switch_to_page(PAGE_MENU);}
        else if(calibration_is_active())calibration_handle_button(key==LVGL_KEY_UP,key==LVGL_KEY_DOWN,key==LVGL_KEY_OK);
        return;
    }
    if(s_page==PAGE_RANGE){
        if(s_edit!=EDIT_NONE){
            switch(key){case LVGL_KEY_UP:_ed_up();break;case LVGL_KEY_DOWN:_ed_dn();break;case LVGL_KEY_OK:_ed_ok();break;case LVGL_KEY_OK_LONG:_exit_edit();break;}return;
        }
        switch(key){
        case LVGL_KEY_UP:s_range_focus=(s_range_focus+RANGE_FOCUS_COUNT-1)%RANGE_FOCUS_COUNT;_apply_range_focus();break;
        case LVGL_KEY_DOWN:s_range_focus=(s_range_focus+1)%RANGE_FOCUS_COUNT;_apply_range_focus();break;
        case LVGL_KEY_OK:
            if(s_range_focus==0)_enter_edit(EDIT_RANGE_V);
            else if(s_range_focus==1)_enter_edit(EDIT_RANGE_I);
            else _switch_to_page(PAGE_MENU); /* 确认返回按钮 */
            break;
        case LVGL_KEY_OK_LONG:_switch_to_page(PAGE_MENU);break;
        }
        return;
    }
    if(s_page==PAGE_INFO){
        if(key==LVGL_KEY_OK||key==LVGL_KEY_OK_LONG)_switch_to_page(PAGE_MENU);
        return;
    }

    if(s_page==PAGE_MAIN){
        if(s_edit!=EDIT_NONE){
            switch(key){case LVGL_KEY_UP:_ed_up();break;case LVGL_KEY_DOWN:_ed_dn();break;case LVGL_KEY_OK:_ed_ok();break;case LVGL_KEY_OK_LONG:_exit_edit();break;}return;
        }
        switch(key){
        case LVGL_KEY_UP:s_focus=(s_focus+FOCUS_COUNT-1)%FOCUS_COUNT;_apply_focus();break;
        case LVGL_KEY_DOWN:s_focus=(s_focus+1)%FOCUS_COUNT;_apply_focus();break;
        case LVGL_KEY_OK:
            switch(s_focus){
            case FOCUS_POWER:if(PowerControl::isPoweredOn()){PowerControl::powerOff();_run_stop();}else{PowerControl::powerOn();_run_start();}break;
            case FOCUS_VSET:_enter_edit(EDIT_V);break;
            case FOCUS_ISET:_enter_edit(EDIT_I);break;
            }break;
        case LVGL_KEY_OK_LONG:_switch_to_page(PAGE_MENU);break;
        }
    }else{
        switch(key){
        case LVGL_KEY_UP:s_menu_idx=(s_menu_idx+MENU_COUNT-1)%MENU_COUNT;_apply_menu_cursor();break;
        case LVGL_KEY_DOWN:s_menu_idx=(s_menu_idx+1)%MENU_COUNT;_apply_menu_cursor();break;
        case LVGL_KEY_OK:
            if(s_menu_idx==0)_switch_to_page(PAGE_RANGE);
            else if(s_menu_idx==1)calibration_start_v();
            else if(s_menu_idx==2)calibration_start_i();
            else if(s_menu_idx==3)_switch_to_page(PAGE_INFO);
            else if(s_menu_idx==4)_switch_to_page(PAGE_MAIN);
            break;
        case LVGL_KEY_OK_LONG:_switch_to_page(PAGE_MAIN);break;
        }
    }
}

static void _apply_focus(void){lv_ui*u=&guider_ui;if(!u->shouye)return;_clear_focus();
    switch(s_focus){
    case FOCUS_POWER:if(u->shouye_btn_1){lv_obj_set_style_border_width(u->shouye_btn_1,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_color(u->shouye_btn_1,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_opa(u->shouye_btn_1,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}break;
    case FOCUS_VSET:if(u->shouye_label_23){lv_obj_set_style_border_width(u->shouye_label_23,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_color(u->shouye_label_23,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_opa(u->shouye_label_23,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shouye_label_23,lv_color_hex(0x1A3344),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}if(u->shouye_label_52)lv_obj_set_style_text_color(u->shouye_label_52,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);break;
    case FOCUS_ISET:if(u->shouye_label_28){lv_obj_set_style_border_width(u->shouye_label_28,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_color(u->shouye_label_28,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_opa(u->shouye_label_28,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shouye_label_28,lv_color_hex(0x1A3344),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}if(u->shouye_label_54)lv_obj_set_style_text_color(u->shouye_label_54,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);break;
    }
}
static void _clear_focus(void){lv_ui*u=&guider_ui;if(!u->shouye)return;
    if(u->shouye_btn_1)lv_obj_set_style_border_width(u->shouye_btn_1,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(u->shouye_label_23){lv_obj_set_style_border_width(u->shouye_label_23,4,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_color(u->shouye_label_23,lv_color_hex(0x2195f6),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_opa(u->shouye_label_23,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
    if(u->shouye_label_28){lv_obj_set_style_border_width(u->shouye_label_28,4,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_color(u->shouye_label_28,lv_color_hex(0x2195f6),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_opa(u->shouye_label_28,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
    if(u->shouye_label_52)lv_obj_set_style_text_color(u->shouye_label_52,lv_color_hex(0x00FF55),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(u->shouye_label_54)lv_obj_set_style_text_color(u->shouye_label_54,lv_color_hex(0x00FF55),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(u->shouye_label_23)lv_obj_set_style_bg_color(u->shouye_label_23,lv_color_hex(0x2194f4),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(u->shouye_label_28)lv_obj_set_style_bg_color(u->shouye_label_28,lv_color_hex(0x2194f4),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
}

static void _apply_range_focus(void){lv_ui*u=&guider_ui;if(!u->shangxianshezhi)return;
    /* 清除所有焦点 */
    if(u->shangxianshezhi_label_2){lv_obj_set_style_border_width(u->shangxianshezhi_label_2,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shangxianshezhi_label_2,lv_color_hex(0x5ca1db),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
    if(u->shangxianshezhi_label_6){lv_obj_set_style_border_width(u->shangxianshezhi_label_6,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shangxianshezhi_label_6,lv_color_hex(0x5ca1db),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
    if(u->shangxianshezhi_btn_1){lv_obj_set_style_border_width(u->shangxianshezhi_btn_1,0,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shangxianshezhi_btn_1,lv_color_hex(0xab00fd),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
    /* 设置新焦点 */
    lv_obj_t*target=(s_range_focus==0)?u->shangxianshezhi_label_2:(s_range_focus==1)?u->shangxianshezhi_label_6:u->shangxianshezhi_btn_1;
    if(target){lv_obj_set_style_border_width(target,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_color(target,lv_color_hex(0xFFFF00),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_opa(target,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(target,lv_color_hex(0x335599),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
}

/* 菜单: 0上限设置 1电压校准 2电流校准 3电源信息 4返回首页 */
static const int MENU_Y[]={26,64,106,147,189};
static void _apply_menu_cursor(void){if(!s_menu_cursor)return;lv_obj_set_pos(s_menu_cursor,30,MENU_Y[s_menu_idx]);lv_obj_move_foreground(s_menu_cursor);lv_obj_clear_flag(s_menu_cursor,LV_OBJ_FLAG_HIDDEN);}

/* MFR_ID 解析 → 品牌名 (与App端 _parseMfrId 一致) */
static const char* _parse_mfr_name(const char* id){
    if(!id||!id[0])return"--";
    /* 台达Delta */
    if(strstr(id,"DELTA")||strstr(id,"DPS-")||strstr(id,"Delta")||strstr(id,"delta"))return"台达Delta";
    /* 雅达Astec */
    if(strstr(id,"ASTEC")||strstr(id,"Astec")||strstr(id,"EMERSON")||strstr(id,"Emerson"))return"雅达Astec";
    /* 雅达Artesyn */
    if(strstr(id,"ARTESYN")||strstr(id,"Artesyn"))return"雅达Artesyn";
    /* Dell PN → 台达Delta */
    if(id[0]>='0'&&id[0]<='9'&&id[1]>='0'&&id[1]<='9'&&id[2]>='A'&&id[2]<='Z')return"台达Delta";
    /* 其他厂商 */
    if(strstr(id,"FLEX")||strstr(id,"Flex"))return"Flex";
    if(strstr(id,"LITEON")||strstr(id,"Liteon"))return"光宝";
    if(strstr(id,"ACBEL")||strstr(id,"Acbel"))return"康舒";
    if(strstr(id,"CHICONY")||strstr(id,"Chicony"))return"群光";
    if(strstr(id,"HIPRO")||strstr(id,"Hipro"))return"高效";
    return id;
}

static void _digits(float v,int d[4]){int ip=(int)v;if(ip<0)ip=0;if(ip>99)ip=99;int fp=(int)((v-ip)*100+0.5f);if(fp>=100){ip++;fp=0;}if(fp<0)fp=0;d[0]=ip/10;d[1]=ip%10;d[2]=fp/10;d[3]=fp%10;}
static float _value(const int d[4]){return(float)(d[0]*10+d[1])+(float)(d[2]*10+d[3])/100.0f;}
static void _enter_edit(edit_t mode){s_edit=mode;s_epos=0;s_blink=true;lv_ui*u=&guider_ui;
    if(mode==EDIT_V&&u->shouye_label_23){lv_obj_set_style_border_width(u->shouye_label_23,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_color(u->shouye_label_23,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_opa(u->shouye_label_23,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shouye_label_23,lv_color_hex(0x331144),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
    if(mode==EDIT_V&&u->shouye_label_52)lv_obj_set_style_text_color(u->shouye_label_52,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(mode==EDIT_I&&u->shouye_label_28){lv_obj_set_style_border_width(u->shouye_label_28,3,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_color(u->shouye_label_28,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_border_opa(u->shouye_label_28,255,(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shouye_label_28,lv_color_hex(0x331144),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}
    if(mode==EDIT_I&&u->shouye_label_54)lv_obj_set_style_text_color(u->shouye_label_54,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);
    if(mode==EDIT_RANGE_V){s_edit_vmax=PowerControl::getVMax();if(u->shangxianshezhi_label_2){lv_obj_set_style_border_color(u->shangxianshezhi_label_2,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shangxianshezhi_label_2,lv_color_hex(0x331144),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}}
    if(mode==EDIT_RANGE_I){s_edit_imax=PowerControl::getIMax();if(u->shangxianshezhi_label_6){lv_obj_set_style_border_color(u->shangxianshezhi_label_6,lv_color_hex(0xFF00FF),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);lv_obj_set_style_bg_color(u->shangxianshezhi_label_6,lv_color_hex(0x331144),(uint32_t)LV_PART_MAIN|(uint32_t)LV_STATE_DEFAULT);}}
}
#define RATED_W 750.0f  /* Dell DPS-750AB 额定功率 */
static void _exit_edit(void){if(s_edit==EDIT_NONE)return;
    if(s_edit==EDIT_RANGE_V){
        PowerControl::setVMax(s_edit_vmax);
        float new_vm=PowerControl::getVMax();
        float im_lim=RATED_W/new_vm;
        if(im_lim<PowerControl::getIMax())PowerControl::setIMax(im_lim);
        if(PowerControl::getSetVoltage()>new_vm)PowerControl::setVoltage(new_vm);
    }
    if(s_edit==EDIT_RANGE_I){
        PowerControl::setIMax(s_edit_imax);
        float new_im=PowerControl::getIMax();
        float vm_lim=RATED_W/new_im;
        if(vm_lim<PowerControl::getVMax())PowerControl::setVMax(vm_lim);
        if(PowerControl::getSetCurrent()>new_im)PowerControl::setCurrent(new_im);
    }
    s_edit=EDIT_NONE;
    if(s_page==PAGE_MAIN)_apply_focus();
    else if(s_page==PAGE_RANGE)_apply_range_focus();
}
static void _ed_show(void){lv_ui*u=&guider_ui;char b[8];int d[4];
    float val=(s_edit==EDIT_V)?PowerControl::getSetVoltage():(s_edit==EDIT_I)?PowerControl::getSetCurrent():(s_edit==EDIT_RANGE_V)?s_edit_vmax:s_edit_imax;
    _digits(val,d);int bk=!s_blink;char c0=(s_epos==0&&bk)?'_':('0'+d[0]),c1=(s_epos==1&&bk)?'_':('0'+d[1]),c2=(s_epos==2&&bk)?'_':('0'+d[2]),c3=(s_epos==3&&bk)?'_':('0'+d[3]);snprintf(b,sizeof(b),"%c%c.%c%c",c0,c1,c2,c3);
    if(s_edit==EDIT_V&&u->shouye_label_52)lv_label_set_text(u->shouye_label_52,b);
    if(s_edit==EDIT_I&&u->shouye_label_54)lv_label_set_text(u->shouye_label_54,b);
    if(s_edit==EDIT_RANGE_V&&u->shangxianshezhi_label_2)lv_label_set_text(u->shangxianshezhi_label_2,b);
    if(s_edit==EDIT_RANGE_I&&u->shangxianshezhi_label_6)lv_label_set_text(u->shangxianshezhi_label_6,b);
}
static void _ed_up(void){int d[4];
    if(s_edit==EDIT_V){_digits(PowerControl::getSetVoltage(),d);d[s_epos]=(d[s_epos]+1)%10;float nv=_value(d);if(nv<=PowerControl::getVMax())PowerControl::setVoltage(nv);}
    if(s_edit==EDIT_I){_digits(PowerControl::getSetCurrent(),d);d[s_epos]=(d[s_epos]+1)%10;float nv=_value(d);if(nv<=PowerControl::getIMax())PowerControl::setCurrent(nv);}
    if(s_edit==EDIT_RANGE_V){_digits(s_edit_vmax,d);d[s_epos]=(d[s_epos]+1)%10;s_edit_vmax=_value(d);}
    if(s_edit==EDIT_RANGE_I){_digits(s_edit_imax,d);d[s_epos]=(d[s_epos]+1)%10;s_edit_imax=_value(d);}
}
static void _ed_dn(void){int d[4];
    if(s_edit==EDIT_V){_digits(PowerControl::getSetVoltage(),d);d[s_epos]=(d[s_epos]+9)%10;float nv=_value(d);if(nv<=PowerControl::getVMax())PowerControl::setVoltage(nv);}
    if(s_edit==EDIT_I){_digits(PowerControl::getSetCurrent(),d);d[s_epos]=(d[s_epos]+9)%10;float nv=_value(d);if(nv<=PowerControl::getIMax())PowerControl::setCurrent(nv);}
    if(s_edit==EDIT_RANGE_V){_digits(s_edit_vmax,d);d[s_epos]=(d[s_epos]+9)%10;s_edit_vmax=_value(d);}
    if(s_edit==EDIT_RANGE_I){_digits(s_edit_imax,d);d[s_epos]=(d[s_epos]+9)%10;s_edit_imax=_value(d);}
}
static void _ed_ok(void){uint32_t now_ms=(uint32_t)(esp_timer_get_time()/1000ULL);if(s_last_ok_ms!=0&&(now_ms-s_last_ok_ms)<DOUBLE_CLICK_MS){s_last_ok_ms=0;_exit_edit();return;}s_last_ok_ms=now_ms;if(s_epos>=3)return;s_epos++;}