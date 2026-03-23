#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "iot_gpio.h"
#include "iot_i2c.h"
#include <unistd.h>
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"

#define OLED_I2C_BAUDRATE  400 * 1000

static void i2c_oled_cn_task(void *arg)
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

    /* 显示英文 "Li Songlun" 在第一行 */
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("Li Songlun", Font_7x10, White);

    /* 显示汉字 李松伦 在第二行（y=20） */
    ssd1306_SetCursor(0, 20);
    ssd1306_DrawChinese(0, Font_16x16, White);  /* 李 */
    ssd1306_DrawChinese(1, Font_16x16, White);  /* 松 */
    ssd1306_DrawChinese(2, Font_16x16, White);  /* 伦 */

    ssd1306_UpdateScreen();
    osDelay(100);
}

static void i2c_oled_cn_entry(void)
{
    osThreadAttr_t attr = {
        .name       = "OledCnTask",
        .stack_size = 4096,
        .priority   = osPriorityNormal,
    };
    osThreadNew(i2c_oled_cn_task, NULL, &attr);
}

SYS_RUN(i2c_oled_cn_entry);
