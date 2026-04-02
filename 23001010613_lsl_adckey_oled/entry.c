#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_adc.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "iot_i2c.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "adckey.h"

#define OLED_I2C_BAUDRATE (400 * 1000)

/* 全局变量：ADC数据和电压值 */
static unsigned short g_adc_data = 0;
static float g_voltage = 0.0f;

/* 当前显示页面：1=学号姓名，2=图片，3=自定义，4=ADC电压 */
static int g_flag_show = 1;

/* ===== OLED 初始化 ===== */
void i2c_oled_init(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_13);
    IoTGpioInit(IOT_IO_NAME_GPIO_14);
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    IoSetPull(IOT_IO_NAME_GPIO_13, IOT_IO_PULL_UP);
    IoSetPull(IOT_IO_NAME_GPIO_14, IOT_IO_PULL_UP);
    IoTI2cInit(0, OLED_I2C_BAUDRATE);
    usleep(20 * 1000);
    ssd1306_Init();
}

/* ===== 第1屏：学号姓名 ===== */
void oled_showInfo(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("23001010613", Font_7x10, White);
    ssd1306_SetCursor(0, 20);
    ssd1306_DrawString("Li Songlun", Font_7x10, White);
    ssd1306_SetCursor(0, 36);
    ssd1306_DrawChinese(0, Font_16x16, White);
    ssd1306_DrawChinese(1, Font_16x16, White);
    ssd1306_DrawChinese(2, Font_16x16, White);
    ssd1306_UpdateScreen();
    osDelay(1);
}

/* ===== 第2屏：图片 ===== */
void oled_showImage(void)
{
    ssd1306_Fill(Black);
    ssd1306_DrawImage(gImage_love, 48, 0, 32, 32);
    ssd1306_SetCursor(0, 36);
    ssd1306_DrawString("23001010613", Font_7x10, White);
    ssd1306_UpdateScreen();
    osDelay(1);
}

/* ===== 第3屏：自定义 ===== */
void oled_showSelfDef(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 10);
    ssd1306_DrawString("Hello HarmonyOS!", Font_7x10, White);
    ssd1306_SetCursor(0, 30);
    ssd1306_DrawString("Li Songlun", Font_7x10, White);
    ssd1306_SetCursor(0, 48);
    ssd1306_DrawString("NEUSOFT 2024", Font_7x10, White);
    ssd1306_UpdateScreen();
    osDelay(1);
}

/* ===== 第4屏：ADC电压值实时显示 ===== */
void oled_showADC(void)
{
    char buf[32];
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("ADC Monitor", Font_7x10, White);
    ssd1306_SetCursor(0, 16);
    snprintf(buf, sizeof(buf), "ADC: %d", g_adc_data);
    ssd1306_DrawString(buf, Font_7x10, White);
    ssd1306_SetCursor(0, 32);
    /* 电压转换：voltage = adc * 1.8 / 4096 */
    int volt_int = (int)(g_voltage);
    int volt_dec = (int)((g_voltage - volt_int) * 1000);
    snprintf(buf, sizeof(buf), "Vol: %d.%03dV", volt_int, volt_dec);
    ssd1306_DrawString(buf, Font_7x10, White);
    ssd1306_UpdateScreen();
    osDelay(1);
}

/* ===== 切屏控制 ===== */
void oled_control(int flag)
{
    if (flag == 1)      oled_showInfo();
    else if (flag == 2) oled_showImage();
    else if (flag == 3) oled_showSelfDef();
    else if (flag == 4) oled_showADC();
}

/* ===== 线程1：按键切屏 ===== */
static void handler_Key_Task(void *arg)
{
    (void)arg;
    int key_val;

    while (1) {
        key_val = readAdCKey1();
        if (key_val == KEY1_PRESS) {
            g_flag_show++;
            if (g_flag_show == 5) g_flag_show = 1;
            printf("key1：向后翻页，当前页面=%d\r\n", g_flag_show);
            oled_control(g_flag_show);
        }

        key_val = readAdCKey2();
        if (key_val == KEY2_PRESS) {
            g_flag_show--;
            if (g_flag_show == 0) g_flag_show = 4;
            printf("key2：向前翻页，当前页面=%d\r\n", g_flag_show);
            oled_control(g_flag_show);
        }

        TaskMsleep(50);
    }
}

/* ===== 线程2：ADC数据采集 ===== */
static void handler_readADC_task(void *arg)
{
    (void)arg;
    unsigned short data = 0;

    while (1) {
        if (hi_adc_read(KEY_ADC_CHANNEL, &data,
            HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) == HI_ERR_SUCCESS) {
            g_adc_data = data;
            g_voltage = (float)data * 1.8f / 4096.0f;
            /* 如果当前在第4屏则实时刷新 */
            if (g_flag_show == 4) {
                oled_showADC();
            }
        }
        osDelay(10); /* 100ms刷新一次 */
    }
}

/* ===== 设备初始化 ===== */
void device_init(void)
{
    i2c_oled_init();
}

/* ===== 入口函数 ===== */
static void demoEntry(void)
{
    device_init();
    oled_control(1);

    osThreadAttr_t attr;

    /* 启动按键线程 */
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

    /* 启动ADC采集线程 */
    attr.name = "handler_readADC_task";
    attr.stack_size = 4096;
    attr.priority = osPriorityBelowNormal;
    if (osThreadNew(handler_readADC_task, NULL, &attr) == NULL) {
        printf("Failed to create handler_readADC_task!\n");
    }
}

SYS_RUN(demoEntry);
