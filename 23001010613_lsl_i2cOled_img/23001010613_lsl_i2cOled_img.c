#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "iot_i2c.h"
#include "iot_gpio.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include <unistd.h>

#define OLED_I2C_BAUDRATE  400 * 1000

static void i2c_oled_img_task(void *arg)
{
    (void)arg;
    IoTGpioInit(IOT_IO_NAME_GPIO_13);
    IoTGpioInit(IOT_IO_NAME_GPIO_14);
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    IoSetPull(IOT_IO_NAME_GPIO_13, IOT_IO_PULL_UP);
    IoSetPull(IOT_IO_NAME_GPIO_14, IOT_IO_PULL_UP);
    IoTI2cInit(0, OLED_I2C_BAUDRATE);
    usleep(20 * 1000);

    ssd1306_Init();
    ssd1306_Fill(Black);

    /* 上半部分：显示华为logo，宽64高30，从x=32开始居中 */
    ssd1306_DrawImage(gImage_logo, 32, 0, 64, 30);

    /* 中间：显示英文名 */
    ssd1306_SetCursor(20, 36);
    ssd1306_DrawString("Li Songlun", Font_7x10, White);

    /* 下方：显示汉字李松伦 */
    ssd1306_SetCursor(16, 48);
    ssd1306_DrawChinese(0, Font_16x16, White);
    ssd1306_DrawChinese(1, Font_16x16, White);
    ssd1306_DrawChinese(2, Font_16x16, White);

    ssd1306_UpdateScreen();
    osDelay(100);
}

static void i2c_oled_img_entry(void)
{
    osThreadAttr_t attr = {
        .name       = "OledImgTask",
        .stack_size = 4096,
        .priority   = osPriorityNormal,
    };
    osThreadNew(i2c_oled_img_task, NULL, &attr);
}

SYS_RUN(i2c_oled_img_entry);
