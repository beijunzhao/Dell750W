/**
 * calibration.h - 6点电压/电流校准模块
 *
 * 校准核心思想：
 *   保存 (目标值, 确认时的PWM, 确认时的ADC) 6组数据
 *   设值: 查表插值 → PWM输出
 *   显示: 查表插值 → 校准电压
 */
#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CALIB_POINTS  6

/* 默认校准目标值 */
extern const float g_calib_v_targets[CALIB_POINTS];  /* {0, 3, 5, 8, 10, 12} */
extern const float g_calib_i_targets[CALIB_POINTS];  /* {0, 10, 20, 30, 50, 62.5} */

/* 校准点数据结构：只存目标值与对应的PWM */
typedef struct {
    float target_v;     /**< 电压目标值 (V) */
    float target_i;     /**< 电流目标值 (A) */
    int   pwm_val;      /**< 确认时的PWM占空比 */
} calib_point_t;

typedef enum { CALIB_TYPE_VOLTAGE = 0, CALIB_TYPE_CURRENT } calib_type_t;
typedef enum { CALIB_STATE_IDLE = 0, CALIB_STATE_ADJUSTING, CALIB_STATE_COMPLETE } calib_state_t;

/* ========== 校准状态机 API ========== */
void calibration_init(void);
void calibration_start_v(void);
void calibration_start_i(void);
void calibration_stop(void);
bool calibration_is_active(void);
calib_type_t calibration_get_type(void);
void calibration_handle_button(bool btn_up, bool btn_down, bool btn_ok);
void calibration_update_adc(int raw_adc);
void calibration_update_pmbus(float current);
const calib_point_t* calibration_get_data(void);
int  calibration_get_current_step(void);
int  calibration_get_pwm(void);
float calibration_get_pmbus_value(void);

/* ========== PWM 校准查表 API ========== */

/**
 * @brief 根据校准表将目标电压映射为 PWM 值
 * @param target_v  目标电压 (V)
 * @return          应输出的 PWM 占空比
 *
 * 查表规则：
 *   - 无校准数据 → 用公式倒算
 *   - target_v <= 0.1V → PWM = 0（关断态）
 *   - 6点分段线性插值
 */
int calibration_v_to_pwm(float target_v);

/**
 * @brief 根据校准表将目标电流映射为 PWM 值
 * @param target_i  目标电流 (A)
 * @return          应输出的 PWM 占空比
 */
int calibration_i_to_pwm(float target_i);

/* ========== ADC 校准查表 API ========== */

/**
 * @brief 6点分段插值: ADC原始读数 → 校准电压
 * @param raw_adc  平滑后的ADC原始值
 * @return         校准后的电压值 (V)
 *
 * 使用电压校准时记录的 (target_v, adc_raw) 6组数据进行分段线性插值。
 * 无校准数据时退回公式计算。
 */
float calculate_calibrated_value(float raw_adc, bool is_voltage);

#ifdef __cplusplus
}
#endif

#endif /* CALIBRATION_H */