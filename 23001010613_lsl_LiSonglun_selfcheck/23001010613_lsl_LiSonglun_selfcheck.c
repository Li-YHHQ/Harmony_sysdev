#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"

void selfcheck(void)
{
    printf("\n23001010613\n");
    printf("Li_Songlun\n");
    printf("Device Self-Check Starting...\n");

    // 初始化 GPIO_10(红)、GPIO_11(绿)、GPIO_12(黄)
    IoTGpioInit(IOT_IO_NAME_GPIO_10);
    IoTGpioInit(IOT_IO_NAME_GPIO_11);
    IoTGpioInit(IOT_IO_NAME_GPIO_12);

    IoSetFunc(IOT_IO_NAME_GPIO_10, IOT_IO_FUNC_GPIO_10_GPIO);
    IoSetFunc(IOT_IO_NAME_GPIO_11, IOT_IO_FUNC_GPIO_11_GPIO);
    IoSetFunc(IOT_IO_NAME_GPIO_12, IOT_IO_FUNC_GPIO_12_GPIO);

    IoTGpioSetDir(IOT_IO_NAME_GPIO_10, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_11, IOT_GPIO_DIR_OUT);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_12, IOT_GPIO_DIR_OUT);

    // 全灭
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE0);

    // 第一阶段：红灯亮2秒，模拟检测Flash存储
    printf("Checking Flash...\n");
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE1);
    osDelay(200);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0);

    // 第二阶段：黄灯亮2秒，模拟检测Wi-Fi芯片
    printf("Checking Wi-Fi...\n");
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE1);
    osDelay(200);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE0);

    // 第三阶段：三灯同时快闪3次，代表自检通过
    printf("Self-Check Passed!\n");
    for (int i = 0; i < 3; i++) {
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE1);
        osDelay(50);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE0);
        osDelay(50);
    }

    // 最后状态：绿灯长亮，设备进入就绪状态
    printf("Device Ready!\n");
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE1);

    while (1) {
        osDelay(1000);
    }
}

SYS_RUN(selfcheck);