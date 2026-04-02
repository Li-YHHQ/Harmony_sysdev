#include <stdio.h>
#include <unistd.h>

#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "adckey.h"

#define LED_GPIO    9   // 核心板LED连接到GPIO9

// LED工作状态
typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON,
    LED_STATE_BLINK,
} LedState;

static void handler_Key_Task(void *arg)
{
    (void)arg;

    uint8_t key_val;
    LedState ledState = LED_STATE_OFF;
    int blinkCounter = 0;
    int ledToggle = 0;

    // 初始化LED GPIO
    IoTGpioInit(LED_GPIO);
    IoSetFunc(LED_GPIO, IOT_IO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(LED_GPIO, IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE1); // 低电平有效，初始熄灭

    while(1){
        key_val = readAdCKey0();
        if(key_val == KEY_PRESS_kernel){
            printf("==>key0 input!\n");
        } else if(key_val == KEY_PRESSLONG_kernel){
            printf("==>key0 long input!\n");
        }

        // S1短按：点亮LED；S1长按：关闭LED
        key_val = readAdCKey1();
        if(key_val == KEY1_PRESS){
            printf("==>S1 short press! LED ON\n");
            ledState = LED_STATE_ON;
        } else if(key_val == KEY1_PRESSLONG){
            printf("==>S1 long press! LED OFF\n");
            ledState = LED_STATE_OFF;
        }

        // S2短按：LED闪烁；S2长按：关闭LED
        key_val = readAdCKey2();
        if(key_val == KEY2_PRESS){
            printf("==>S2 short press! LED BLINK\n");
            ledState = LED_STATE_BLINK;
            blinkCounter = 0;
        } else if(key_val == KEY2_PRESSLONG){
            printf("==>S2 long press! LED OFF\n");
            ledState = LED_STATE_OFF;
        }

        // LED状态控制（低电平有效：VALUE0=亮，VALUE1=灭）
        switch(ledState){
            case LED_STATE_ON:
                IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE0);
                break;
            case LED_STATE_OFF:
                IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE1);
                break;
            case LED_STATE_BLINK:
                blinkCounter++;
                if(blinkCounter >= 10){   // 每500ms（10×50ms）翻转一次
                    blinkCounter = 0;
                    ledToggle = !ledToggle;
                    IoTGpioSetOutputVal(LED_GPIO,
                        ledToggle ? IOT_GPIO_VALUE0 : IOT_GPIO_VALUE1);
                }
                break;
            default:
                break;
        }

        TaskMsleep(50);/* 睡眠50毫秒 */
    }
}


// 入口函数
static void demoEntry(void)
{
    // 定义线程属性
    osThreadAttr_t attr;
    attr.name = "handler_Key_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = osPriorityNormal;

    // 创建线程
    if (osThreadNew(handler_Key_Task, NULL, &attr) == NULL)
    {
        printf("Failed to create handler_Key_Task!\n");
    }
}

// 运行入口函数
SYS_RUN(demoEntry);
