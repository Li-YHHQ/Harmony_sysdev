#include "ssd1306/ssd1306.h"
#include "ssd1306/ssd1306_fonts.h"
#include "iot_i2c.h"
#include "iot_gpio.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include <unistd.h>

#define OLED_I2C_BAUDRATE  (400 * 1000)
#define GPIO_RED    IOT_IO_NAME_GPIO_10
#define GPIO_GREEN  IOT_IO_NAME_GPIO_11

/* OLED显示汉字封装：根据state显示对应文字 */
/* state=0: 红灯亮绿灯灭  state=1: 红灯灭绿灯亮  state=2: 红灯灭绿灯灭 */
static void oled_show_state(uint8_t state)
{
    ssd1306_Fill(Black);
    ssd1306_DrawImage(gImage_love, 96, 32, 32, 32);
    ssd1306_SetCursor(0, 24);
    /* 红 */
    ssd1306_DrawChinese(0, Font_16x16_traffic, White);
    /* 灯 */
    ssd1306_DrawChinese(2, Font_16x16_traffic, White);
    if (state == 0) {
        /* 亮 */
        ssd1306_DrawChinese(3, Font_16x16_traffic, White);
    } else {
        /* 灭 */
        ssd1306_DrawChinese(4, Font_16x16_traffic, White);
    }

    ssd1306_SetCursor(0, 42);
    /* 绿 */
    ssd1306_DrawChinese(1, Font_16x16_traffic, White);
    /* 灯 */
    ssd1306_DrawChinese(2, Font_16x16_traffic, White);
    if (state == 1) {
        /* 亮 */
        ssd1306_DrawChinese(3, Font_16x16_traffic, White);
    } else {
        /* 灭 */
        ssd1306_DrawChinese(4, Font_16x16_traffic, White);
    }

    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("23001010613", Font_7x10, White);
    ssd1306_UpdateScreen();
}

static void traffic_task(void *arg)
{
    (void)arg;

    /* GPIO 初始化 */
    IoTGpioInit(IOT_IO_NAME_GPIO_13);
    IoTGpioInit(IOT_IO_NAME_GPIO_14);
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    IoSetPull(IOT_IO_NAME_GPIO_13, IOT_IO_PULL_UP);
    IoSetPull(IOT_IO_NAME_GPIO_14, IOT_IO_PULL_UP);

    /* LED GPIO 初始化 */
    IoTGpioInit(GPIO_RED);
    IoTGpioSetDir(GPIO_RED, IOT_GPIO_DIR_OUT);
    IoTGpioInit(GPIO_GREEN);
    IoTGpioSetDir(GPIO_GREEN, IOT_GPIO_DIR_OUT);

    /* I2C + OLED 初始化 */
    IoTI2cInit(0, OLED_I2C_BAUDRATE);
    usleep(20 * 1000);
    ssd1306_Init();

    /* 循环10次 */
    for (int i = 0; i < 10; i++) {
        /* 红灯亮，绿灯灭 */
        IoTGpioSetOutputVal(GPIO_RED,   IOT_GPIO_VALUE1);
        IoTGpioSetOutputVal(GPIO_GREEN, IOT_GPIO_VALUE0);
        oled_show_state(0);
        osDelay(300); /* 3秒 */

        /* 绿灯亮，红灯灭 */
        IoTGpioSetOutputVal(GPIO_RED,   IOT_GPIO_VALUE0);
        IoTGpioSetOutputVal(GPIO_GREEN, IOT_GPIO_VALUE1);
        oled_show_state(1);
        osDelay(300); /* 3秒 */
    }

    /* 全灭 */
    IoTGpioSetOutputVal(GPIO_RED,   IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(GPIO_GREEN, IOT_GPIO_VALUE0);
    oled_show_state(2);
    osDelay(100); /* 1秒后熄屏 */
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
}

static void traffic_entry(void)
{
    osThreadAttr_t attr = {
        .name       = "TrafficTask",
        .stack_size = 4096,
        .priority   = osPriorityNormal,
    };
    osThreadNew(traffic_task, NULL, &attr);
}

SYS_RUN(traffic_entry);
