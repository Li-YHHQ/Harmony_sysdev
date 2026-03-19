#include "ssd1306.h"
#include "hi_i2c.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"

#define HI_I2C_IDX_0       0
#define OLED_I2C_BAUDRATE  400 * 1000

static void i2c_oled_control(void *arg)
{
    (void)arg;
    // 初始化GPIO
    IoTGpioInit(IOT_IO_NAME_GPIO_13);
    IoTGpioInit(IOT_IO_NAME_GPIO_14);
    // 设置GPIO_13的复用功能为i2c sda
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    // 设置GPIO_14的复用功能为i2c scl
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    // 设置波特率
    hi_i2c_init(HI_I2C_IDX_0, OLED_I2C_BAUDRATE);
    // 等待0.02秒
    usleep(20 * 1000);
    // ssd1306芯片初始化
    ssd1306_Init();
    // Black 表示息屏，所有像素点都不点亮
    ssd1306_Fill(Black);
    // 设置文字显示起始位置，横轴0~127，纵轴0~63
    ssd1306_SetCursor(0, 32);
    // 输出文字，字体宽7像素高10像素，White为点亮
    ssd1306_DrawString("Hello OpenHarmony!", Font_7x10, White);
    // 更新屏幕
    ssd1306_UpdateScreen();
    // 等待一点时间
    osDelay(100);
}

SYS_RUN(i2c_oled_control);
