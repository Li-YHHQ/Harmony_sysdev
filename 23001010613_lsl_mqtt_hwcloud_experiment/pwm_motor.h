#ifndef PWM_MOTOR_H
#define PWM_MOTOR_H

#include <stdint.h>

// 电机方向枚举
typedef enum {
    MOTOR_DIR_CW=1,     // 顺时针
    MOTOR_DIR_CCW       // 逆时针
} MotorDir;

// 按方向初始化PWM与GPIO复用关系。
// dir: 电机方向，取值为 MOTOR_DIR_CW 或 MOTOR_DIR_CCW。
void pwm_motor_init(MotorDir dir);

// 启动电机并设置转向和占空比。
// dir: 目标方向；duty: 占空比(0~100)。
void pwm_motor_start(MotorDir dir, int8_t duty);

// 停止当前方向对应的PWM输出。
void pwm_motor_stop(void);

// 获取当前记录的电机方向。
MotorDir pwm_motor_get_dir(void); 

// 获取当前记录的占空比(0~100)。
int8_t   pwm_motor_get_duty(void);


#endif
