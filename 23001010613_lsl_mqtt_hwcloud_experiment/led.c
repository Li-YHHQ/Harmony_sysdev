/*
 * led.c — GPIO9 LED 控制实现
 * GPIO9 与蜂鸣器共用引脚，此文件仅处理普通GPIO模式（LED）。
 * 低电平亮（IOT_GPIO_VALUE0），高电平灭（IOT_GPIO_VALUE1）。
 */

#include "ohos_init.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "led.h"

/* LED 使用的 GPIO 编号 */
#define LED_GPIO_PIN   9

/*
 * led_init — 初始化GPIO9为普通GPIO输出模式
 * 调用时机：device_init() 中，buzzer_pwm_init() 之前
 * 注意：buzzer_pwm_init() 会将GPIO9切换为PWM模式，
 *       buzzer_pwm_stop() 会将其恢复为GPIO模式，与led_off()效果一致
 */
void led_init(void)
{
    /* 初始化GPIO9 */
    IoTGpioInit(LED_GPIO_PIN);
    /* 设为普通GPIO功能（功能码0 = IOT_IO_FUNC_GPIO_9_GPIO）*/
    IoSetFunc(LED_GPIO_PIN, IOT_IO_FUNC_GPIO_9_GPIO);
    /* 设为输出方向 */
    IoTGpioSetDir(LED_GPIO_PIN, IOT_GPIO_DIR_OUT);
    /* 初始状态：熄灭（高电平）*/
    IoTGpioSetOutputVal(LED_GPIO_PIN, IOT_GPIO_VALUE1);
}

/*
 * led_on — 点亮LED
 * 输出低电平（VALUE0），LED导通发光
 */
void led_on(void)
{
    IoTGpioSetOutputVal(LED_GPIO_PIN, IOT_GPIO_VALUE0);
}

/*
 * led_off — 熄灭LED
 * 输出高电平（VALUE1），LED截止熄灭
 */
void led_off(void)
{
    IoTGpioSetOutputVal(LED_GPIO_PIN, IOT_GPIO_VALUE1);
}
