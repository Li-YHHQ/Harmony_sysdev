#include <stdio.h>
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "key.h"

#define LED_GPIO IOT_IO_NAME_GPIO_9

static void handler_Key_Task(void *arg)
{
    (void)arg;
    int led_state = 0;

    key_init();

    IoTGpioInit(LED_GPIO);
    IoSetFunc(LED_GPIO, IOT_IO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(LED_GPIO, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE0);

    while (1) {
        int key_val = readKey();

        if (key_val == KEY_PRESS_kernel) {
            led_state = !led_state;
            IoTGpioSetOutputVal(LED_GPIO, led_state ? IOT_GPIO_VALUE1 : IOT_GPIO_VALUE0);
            printf("短按：LED %s\n", led_state ? "亮" : "灭");
        } else if (key_val == KEY_PRESSLONG_kernel) {
            printf("长按：LED闪烁\n");
            for (int i = 0; i < 3; i++) {
                IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE1);
                osDelay(10);
                IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE0);
                osDelay(10);
            }
            IoTGpioSetOutputVal(LED_GPIO, led_state ? IOT_GPIO_VALUE1 : IOT_GPIO_VALUE0);
        }
        osDelay(1);
    }
}

static void demoEntry(void)
{
    osThreadAttr_t attr;
    attr.name = "handler_Key_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    if (osThreadNew(handler_Key_Task, NULL, &attr) == NULL) {
        printf("Failed to create handler_Key_Task!\n");
    }
}

SYS_RUN(demoEntry);
