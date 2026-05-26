/**
 * calibration.h - ADC/PWM 校准状态机与 UI
 *
 * 校准流程 (6 步):
 *   1. 用户通过 UP/DOWN 调节 PWM 占空比, 使万用表读数等于目标电压
 *   2. 按 OK 确认, 记录当前 PWM 和 ADC 值
 *   3. 进入下一步, 直到 6 个点全部完成
 *   4. 完成时自动计算校准参数并保存到 NVS
 *
 * 外部接口:
 *   - calibration_start(): 进入校准模式
 *   - calibration_stop():  退出校准模式
 *   - calibration_is_active(): 是否正在校准
 *   - calibration_handle_button(): 按键处理 (UP/DOWN/OK)
 *   - calibration_update_adc(int raw_adc): 更新实时 ADC 读数
 */
#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 校准点数量 ---- */
#define CALIB_POINTS  6

/* ---- 目标电压数组 ---- */
extern const float g_calib_targets[CALIB_POINTS];

/* ---- 校准数据结构体 ---- */
typedef struct {
    float target;   /**< 目标电压 (V) */
    int   pwm_val;  /**< 确认时的 PWM 占空比 */
    int   adc_val;  /**< 确认时的原始 ADC 读数 */
} calib_data_t;

/* ---- 校准状态 ---- */
typedef enum {
    CALIB_STATE_IDLE = 0,       /**< 未在校准模式 */
    CALIB_STATE_ADJUSTING,      /**< 正在调节 PWM */
    CALIB_STATE_COMPLETE,       /**< 全部 6 点完成 */
} calib_state_t;

/* ========== API ========== */

/**
 * @brief 初始化校准模块 (注册 LVGL 样式等)
 */
void calibration_init(void);

/**
 * @brief 进入校准模式
 * - 创建校准 UI 界面
 * - 将状态机设为 Step 0
 * - 设置 PWM 为当前电压对应的值作为初始值
 */
void calibration_start(void);

/**
 * @brief 退出校准模式
 * - 销毁校准 UI 界面
 * - 恢复主界面
 */
void calibration_stop(void);

/** @brief 是否正在校准中 */
bool calibration_is_active(void);

/**
 * @brief 按键处理 (由 UI 任务循环调用)
 * @param btn_up    UP 按键按下
 * @param btn_down  DOWN 按键按下
 * @param btn_ok    OK 按键按下
 */
void calibration_handle_button(bool btn_up, bool btn_down, bool btn_ok);

/**
 * @brief 更新实时 ADC 读数 (由硬件轮询任务调用)
 * @param raw_adc  原始 ADC 读数 (0~4095)
 */
void calibration_update_adc(int raw_adc);

/**
 * @brief 获取当前校准数据指针
 */
const calib_data_t* calibration_get_data(void);

/**
 * @brief 获取当前校准步骤 (0-based, 0~5)
 * @return 当前步骤, 未在校准模式时返回 -1
 */
int calibration_get_current_step(void);

#ifdef __cplusplus
}
#endif

#endif /* CALIBRATION_H */
