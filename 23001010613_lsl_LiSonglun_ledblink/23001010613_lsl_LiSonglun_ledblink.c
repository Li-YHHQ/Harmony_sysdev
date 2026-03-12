#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"

#define DELAY_MS 300  // 300*10ms=3s

void ledblink(void)
{
    printf("\n23001010613\n");
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
        // 红亮，绿灭，黄灭
        IoTGpioSetOutputVal(10, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(11, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(12, IOT_GPIO_VALUE0);
        osDelay(DELAY_MS);

        // 红灭，绿亮，黄灭
        IoTGpioSetOutputVal(10, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(11, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(12, IOT_GPIO_VALUE0);
        osDelay(DELAY_MS);

        // 红灭，绿灭，黄亮
        IoTGpioSetOutputVal(10, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(11, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(12, IOT_GPIO_VALUE1);
        osDelay(DELAY_MS);
    }
}

SYS_RUN(ledblink);