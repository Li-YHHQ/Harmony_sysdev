#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"

#define DELAY_MS 100  // osDelay单位是10ms，100*10ms=1s

void blueled_blink(void)
{
    printf("\nlsl23001010613\n");
    printf("Li_Songlun\n");

    // 初始化GPIO_10(红)、GPIO_11(绿)、GPIO_12(黄)
    IoTGpioInit(10);
    IoTGpioInit(11);
    IoTGpioInit(12);

    // 设置为普通GPIO模式
    IoSetFunc(10, 0);
    IoSetFunc(11, 0);
    IoSetFunc(12, 0);

    // 设置为输出方向
    IoTGpioSetDir(10, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(11, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(12, IOT_GPIO_DIR_OUT);

    while (1) {
        // 三灯同时点亮
        IoTGpioSetOutputVal(10, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(11, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(12, IOT_GPIO_VALUE1);
        osDelay(DELAY_MS);

        // 三灯同时熄灭
        IoTGpioSetOutputVal(10, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(11, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(12, IOT_GPIO_VALUE0);
        osDelay(DELAY_MS);
    }
}

SYS_RUN(blueled_blink);