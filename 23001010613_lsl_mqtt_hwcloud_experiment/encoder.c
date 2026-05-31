/* encoder.c — 编码器驱动实现
 * 硬件：A相=GPIO_7（中断输入），B相=GPIO_6（方向判断输入）
 * 速度公式：speed(m/s) = 0.16 * delta_pulse / 0.1
 *           每 100ms 由 handler_encoder_Task 计算一次，计算后清零 delta。
 */
#include "encoder.h"
#include "iot_gpio.h"       /* IoTGpioInit / IoTGpioSetDir / IoTGpioGetInputVal */
#include "iot_gpio_ex.h"    /* IOT_IO_NAME_GPIO_x / IOT_IO_FUNC_xxx */
#include "hi_io.h"          /* hi_io_set_func / hi_io_set_pull / HI_IO_NAME_GPIO_x */
#include "cmsis_os2.h"
#include <stdio.h>

/* 主模块定义的编码器全局变量（volatile防止编译器优化） */
extern volatile int g_encoder_pulse_total; /* 总脉冲数（累计，不清零） */
extern volatile int g_encoder_pulse_delta; /* 增量脉冲数（每100ms清零） */
extern volatile int g_encoder_dir;         /* 当前方向：0=CW正转，1=CCW反转 */

/* A相上升沿中断回调
 * 同时读取B相电平判断旋转方向：
 *   B=HIGH → CW（正转）；B=LOW → CCW（反转）
 */
static void Encoder_IRQHandler(char *arg)
{
    (void)arg; /* 未使用参数消除警告 */

    IotGpioValue level = IOT_GPIO_VALUE0;
    /* 读取 B相（GPIO_6）当前电平 */
    IoTGpioGetInputVal(6, &level);

    if (level == IOT_GPIO_VALUE1) {
        g_encoder_dir = 0; /* B相高电平：A超前B → 正转 CW */
    } else {
        g_encoder_dir = 1; /* B相低电平：B超前A → 反转 CCW */
    }

    g_encoder_pulse_total++; /* 累计总脉冲 */
    g_encoder_pulse_delta++; /* 增量脉冲（供速度计算，100ms清零） */
}

/* encoder_init — 初始化编码器双相 GPIO */
void encoder_init(void)
{
    /* ——— A相：GPIO_7，上升沿中断 ——— */
    IoTGpioInit(7);
    /* 复用为普通 GPIO 模式 */
    hi_io_set_func(HI_IO_NAME_GPIO_7, HI_IO_FUNC_GPIO_7_GPIO);
    /* 上拉，确保无信号时为高电平 */
    hi_io_set_pull(HI_IO_NAME_GPIO_7, HI_IO_PULL_UP);
    /* 设为输入 */
    IoTGpioSetDir(7, IOT_GPIO_DIR_IN);
    /* 注册上升沿中断，回调 Encoder_IRQHandler */
    IoTGpioRegisterIsrFunc(7, IOT_INT_TYPE_EDGE,
                           IOT_GPIO_EDGE_RISE_LEVEL_HIGH,
                           Encoder_IRQHandler, NULL);

    /* ——— B相：GPIO_6，仅作输入（在中断里读取） ——— */
    IoTGpioInit(6);
    hi_io_set_func(HI_IO_NAME_GPIO_6, HI_IO_FUNC_GPIO_6_GPIO);
    hi_io_set_pull(HI_IO_NAME_GPIO_6, HI_IO_PULL_UP);
    IoTGpioSetDir(6, IOT_GPIO_DIR_IN);

    printf("encoder_init done\n");
}
