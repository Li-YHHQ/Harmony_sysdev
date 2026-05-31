#include <stdio.h>

#include "iot_gpio.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_systick.h"
#include "hi_adc.h"
#include "hi_io.h"
#include "iot_pwm.h"
#include "hi_time.h"
#include "iot_errno.h"
#include "iot_gpio_ex.h"
#include "pwm_motor.h"

// PWM 频率分频值：值越大，输出频率越低
#define PWM_FREQ_DIVISION 8000

// PWM 通道与引脚映射
#define IOT_PWM_PORT_PWM2 2 // GPIO_02
#define IOT_PWM_PORT_PWM4 4 // GPIO_01

// ==================== 全局变量 ====================
int8_t g_motor_dir = 0;  // 该值初始化为一个非法值，之后会被正确设置，当前电机方向
int8_t g_motor_duty = 0; // 当前占空比

static void motor_pin_to_gpio_low(unsigned int gpio)
{
    IoTGpioInit(gpio);
    if (gpio == IOT_IO_NAME_GPIO_1) {
        IoSetFunc(gpio, IOT_IO_FUNC_GPIO_1_GPIO);
    } else {
        IoSetFunc(gpio, IOT_IO_FUNC_GPIO_2_GPIO);
    }
    IoTGpioSetDir(gpio, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(gpio, IOT_GPIO_VALUE0);
}

static void pwm_motor_hw_reset(void)
{
    IoTPwmStop(IOT_PWM_PORT_PWM4);
    IoTPwmStop(IOT_PWM_PORT_PWM2);
    motor_pin_to_gpio_low(IOT_IO_NAME_GPIO_1);
    motor_pin_to_gpio_low(IOT_IO_NAME_GPIO_2);
}

// ==================== 方向设置 ====================
void pwm_motor_set_dir(MotorDir dir)
{
    // 正确的合法性判断
    if (dir != MOTOR_DIR_CW && dir != MOTOR_DIR_CCW)
    {
        printf("==》错误：电机方向无效！\r\n");
        return;
    }

    g_motor_dir = dir;

    if (dir == MOTOR_DIR_CW)
    {
        printf("==》电机方向：顺时针 dir=%d\r\n", dir);
    }
    else
    {
        printf("==》电机方向：逆时针 dir=%d\r\n", dir);
    }
}

// ==================== 占空比设置 ====================
void pwm_motor_set_duty(int8_t duty)
{
    // 占空比统一限制在 0~100，避免传入越界值
    if (duty < 0)
        duty = 0;
    if (duty > 100)
        duty = 100;
    g_motor_duty = duty;
    printf("电机duty=%d\n", g_motor_duty);
}

// ==================== 获取函数 ====================
MotorDir pwm_motor_get_dir(void) { return g_motor_dir; }//获取当前方向
int8_t pwm_motor_get_duty(void) { return g_motor_duty; }//获取当前占空比

// ==================== 统一初始化 ====================
void pwm_motor_init(MotorDir dir)
{
    pwm_motor_hw_reset();

    // H 桥驱动方式：一侧输出 PWM，另一侧固定拉低
    if (dir == MOTOR_DIR_CW)
    {
        // 顺时针：GPIO1 输出 PWM
        IoTGpioInit(IOT_IO_NAME_GPIO_1);
        IoSetFunc(IOT_IO_NAME_GPIO_1, IOT_IO_FUNC_GPIO_1_PWM4_OUT);
        IoTPwmInit(IOT_PWM_PORT_PWM4);

        // GPIO2 拉低
        IoTGpioInit(IOT_IO_NAME_GPIO_2);
        IoSetFunc(IOT_IO_NAME_GPIO_2, IOT_IO_FUNC_GPIO_2_GPIO);
        IoTGpioSetDir(IOT_IO_NAME_GPIO_2, IOT_GPIO_DIR_OUT);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_2, IOT_GPIO_VALUE0);
    }
    else
    {
        // 逆时针：GPIO2 输出 PWM
        IoTGpioInit(IOT_IO_NAME_GPIO_2);
        IoSetFunc(IOT_IO_NAME_GPIO_2, IOT_IO_FUNC_GPIO_2_PWM2_OUT);
        IoTPwmInit(IOT_PWM_PORT_PWM2);

        // GPIO1 拉低
        IoTGpioInit(IOT_IO_NAME_GPIO_1);
        IoSetFunc(IOT_IO_NAME_GPIO_1, IOT_IO_FUNC_GPIO_1_GPIO);
        IoTGpioSetDir(IOT_IO_NAME_GPIO_1, IOT_GPIO_DIR_OUT);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_1, IOT_GPIO_VALUE0);
    }

    // 记录当前方向，供 start/stop 使用
    pwm_motor_set_dir(dir);
}

// ==================== 启动电机 ====================
void pwm_motor_start(MotorDir dir, int8_t duty)
{
    // 每次启动都按目标方向重新配置，避免切向后残留旧复用状态
    pwm_motor_init(dir);

    // 设置占空比
    pwm_motor_set_duty(duty);

    // 在当前方向对应的通道上启动 PWM
    if (dir == MOTOR_DIR_CW)
    {
        IoTPwmStart(IOT_PWM_PORT_PWM4, g_motor_duty, PWM_FREQ_DIVISION);
    }
    else
    {
        IoTPwmStart(IOT_PWM_PORT_PWM2, g_motor_duty, PWM_FREQ_DIVISION);
    }
}

// ==================== 停止电机 ====================
void pwm_motor_stop(void)
{
    pwm_motor_hw_reset();
    // 停止后将占空比状态清零
    g_motor_duty = 0;
}
