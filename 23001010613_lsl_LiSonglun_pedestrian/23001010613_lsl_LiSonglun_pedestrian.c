#include <stdio.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"

void pedestrian_light(void)
{
    printf("\n23001010613\n");
    printf("Li_Songlun\n");
    printf("Pedestrian Crossing Light\n");

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

    while (1) {
        // 第一阶段：绿灯亮3秒，行人通行
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0); // 红灭
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE1); // 绿亮
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE0); // 黄灭
        osDelay(300);

        // 第二阶段：绿灯快闪3次，提示即将禁行
        for (int i = 0; i < 3; i++) {
            IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE0); // 绿灭
            osDelay(30);
            IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE1); // 绿亮
            osDelay(30);
        }
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE0); // 绿灭

        // 第三阶段：黄灯亮2秒，过渡警示
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE1); // 黄亮
        osDelay(200);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE0); // 黄灭

        // 第四阶段：红灯亮3秒，禁止通行
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE1); // 红亮
        osDelay(300);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0); // 红灭
    }
}

SYS_RUN(pedestrian_light);