#include <stdio.h>
#include <unistd.h>

#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"

#define DELAY_MS 300

void led_control(void)
{
    printf("\nlsl23001010613\n");
    printf("Li_Songlun\n");
    IoTGpioInit(IOT_IO_NAME_GPIO_9);
    IoSetFunc(IOT_IO_NAME_GPIO_9, IOT_IO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
    while (1) {
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_9, IOT_GPIO_VALUE1);
        osDelay(DELAY_MS);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_9, IOT_GPIO_VALUE0);
        osDelay(DELAY_MS);
    }
}

SYS_RUN(led_control);