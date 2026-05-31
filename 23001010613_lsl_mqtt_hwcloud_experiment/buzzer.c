#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "iot_pwm.h"
#include "iot_errno.h"
#include "iot_gpio_ex.h"

#define PWM_FREQ_DIVITION 4000
#define IOT_PWM_PORT_PWM0 0

void buzzer_pwm_init(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_9);
    IoSetFunc(IOT_IO_NAME_GPIO_9, IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    IoTGpioSetDir(HI_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
    IoTPwmInit(IOT_PWM_PORT_PWM0);
}

void buzzer_pwm_start(int duty)
{
    buzzer_pwm_init();
    if ((duty <= 0) || (duty >= 100)) return;
    IoTPwmStart(IOT_PWM_PORT_PWM0, duty, PWM_FREQ_DIVITION);
}

void buzzer_pwm_start1(int duty, int freq)
{
    buzzer_pwm_init();
    if ((duty <= 0) || (duty >= 100)) return;
    IoTPwmStart(IOT_PWM_PORT_PWM0, duty, freq);
}

void buzzer_pwm_stop(void)
{
    IoTPwmStop(IOT_PWM_PORT_PWM0);
    IoSetFunc(IOT_IO_NAME_GPIO_9, IOT_IO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_9, IOT_GPIO_VALUE1);
}

void buzzer_gpio_init(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_9);
    IoSetFunc(IOT_IO_NAME_GPIO_9, IOT_IO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
}

void buzzer_gpio_start(void)
{
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_9, IOT_GPIO_VALUE0);
}

void buzzer_gpio_stop(void)
{
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_9, IOT_GPIO_VALUE1);
}
