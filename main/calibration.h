/**
 * calibration.h - 6点校准模块（电压ADC + 电流PMBus）
 *
 * 电压校准: V_PWM(LEDC_CH0) + ADC → 6点分段线性插值
 * 电流校准: I_PWM(LEDC_CH1) + PMBus I_out → 6点查表修正映射
 */
#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CALIB_POINTS  6

/* ---- 默认目标值数组（当 getVMax/getIMax 异常时回退） ---- */
extern const float g_calib_v_targets[CALIB_POINTS];
extern const float g_calib_i_targets[CALIB_POINTS];

/* ---- 6点校准数据结构 ---- */
typedef struct {
    float target_v;        /**< 电压目标值 (V) */
    float target_i;        /**< 电流目标值 (A) */
    int   adc_raw;         /**< 电压校准时ADC原始读数 */
    float pmbus_i_raw;     /**< 电流校准时PMBus原始电流值 */
    int   pwm_val;         /**< 确认时的PWM占空比 */
} calib_point_t;

/* ---- 校准类型 ---- */
typedef enum {
    CALIB_TYPE_VOLTAGE = 0,
    CALIB_TYPE_CURRENT,
} calib_type_t;

/* ---- 校准状态 ---- */
typedef enum {
    CALIB_STATE_IDLE = 0,
    CALIB_STATE_ADJUSTING,
    CALIB_STATE_COMPLETE,
} calib_state_t;

/* ========== 校准状态机 API ========== */

void calibration_init(void);
void calibration_start_v(void);         /* 电压校准 */
void calibration_start_i(void);         /* 电流校准 */
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

/* ========== 6点插值计算 API ========== */

/**
 * @brief 执行分段线性插值计算（统一API）
 * @param input         当前的原始输入值 (ADC raw 或 PMBus raw)
 * @param table         校准点数组
 * @param num_points    校准点数量 (6)
 * @param is_voltage    电压(true)用adc_raw，电流(false)用pmbus_i_raw
 * @return float        计算出的标定值
 *
 * 规则:
 *   - input <= table[0].raw → 返回 0.0
 *   - 分段线性插值
 *   - 外推用最后两点斜率
 */
float calculate_calibrated_value(float input, const calib_point_t* table, int num_points, bool is_voltage);

/** @brief 6点电压校准插值（兼容桥接，调用 calculate_calibrated_value） */
float calculate_calibrated_v(int adc_raw, const calib_point_t* points);

/** @brief 6点电流校准插值（兼容桥接，调用 calculate_calibrated_value） */
float calculate_calibrated_i(float pmbus_raw, const calib_point_t* points);

#ifdef __cplusplus
}
#endif

#endif /* CALIBRATION_H */