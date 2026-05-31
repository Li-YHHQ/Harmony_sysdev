/* main_entry.c — 智能座舱车辆状态监控系统主入口
 * 23001010613 李松伦
 *
 * 功能：
 *   - OLED 6屏循环显示：
 *       0=个人信息  1=温湿度  2=阈值调整
 *       3=仪表盘（方案A）  4=安全评分（方案B）  5=云命令记录
 *   - AHT20 温湿度采集 + 超限报警（蜂鸣器+LED）
 *   - ADC 按键控制屏幕切换、阈值调整、电机控制
 *   - 编码器测速（每100ms计算一次）
 *   - 华为云 MQTT 数据上报（含安全评分）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* math.h 不在链接路径内，三角函数改用查找表实现（见 oled_showMotor） */
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_i2c.h"        /* IoTI2cInit */
#include "iot_gpio_ex.h"    /* IoSetFunc / IoSetPull / IOT_IO_NAME_GPIO_x */
#include "hi_io.h"          /* hi_io_set_func（encoder.c 需要，main 里也用） */
#include "aht20.h"          /* i2c_aht20_init / aht20_getDat */
#include "buzzer.h"         /* buzzer_pwm_start1 / buzzer_pwm_stop */
#include "led.h"            /* led_init / led_on / led_off */
#include "adckey.h"         /* readAdCKey0/1/2 / KEY_* 宏 */
#include "pwm_motor.h"      /* MOTOR_DIR_CW / MOTOR_DIR_CCW / pwm_motor_* */
#include "encoder.h"        /* encoder_init */
#include "ssd1306/ssd1306.h"
#include "ssd1306/ssd1306_fonts.h"

/* ====================================================================
 * 外部函数声明（分别在 wifi_entry.c / hw_cloud_entry.c 中定义）
 * ==================================================================== */
extern void WifiTask(void);
extern int  huawei_cloud_mqtt_init(void);
extern void deal_report_msg(void);

/* ====================================================================
 * 全局变量定义
 * ==================================================================== */

/* 温湿度（由 TempHumiTask 更新，deal_report_msg 上报） */
float g_temperature = 0.0f;
float g_humidity    = 0.0f;

/* 温度阈值（第3屏可通过按键调整） */
float g_temp_upLimit = 35.0f;  /* 上限阈值，默认35℃ */
float g_temp_doLimit = 15.0f;  /* 下限阈值，默认15℃ */

/* 蜂鸣器屏蔽标志：1=屏蔽（不报警），0=正常 */
uint8_t g_flag_buzzer_disable = 0;

/* 当前显示屏号：0~5 对应6个显示屏 */
int g_flag_show = 0;

/* 第3屏阈值调整选中项：0=上限，1=下限 */
uint8_t g_flag_threshold_select = 0;

/* 电机相关：变量定义在 pwm_motor.c，此处用 extern 引用，避免重复定义 */
extern int8_t g_motor_duty; /* 电机占空比（0/20/50/80），pwm_motor.c 定义 */
extern int8_t g_motor_dir;  /* 电机方向（MOTOR_DIR_CW/CCW），pwm_motor.c 定义 */

/* 编码器相关（中断中写，任务中读） */
volatile int g_encoder_pulse_total = 0; /* 总脉冲数（累计，不清零） */
volatile int g_encoder_pulse_delta = 0; /* 增量脉冲数（每100ms清零） */
volatile int g_encoder_dir = 0;         /* 当前方向：0=CW，1=CCW */
float g_speed = 0.0f;                   /* 当前车速(m/s) */

/* 速度历史环形缓冲区（供仪表盘折线图使用） */
#define SPEED_HISTORY_SIZE 24
float g_speed_history[SPEED_HISTORY_SIZE] = {0};
int   g_speed_history_idx = 0;

/* 华为云命令记录（第6屏显示） */
char g_cloud_cmd_line1[32] = "waiting...";
char g_cloud_cmd_line2[32] = "";
char g_cloud_cmd_line3[32] = "";

/* MY_PI 已不需要：仪表盘改用 sin_lut 查找表 */

/* ====================================================================
 * 方案B：安全评分计算（非 static，hw_cloud_entry.c 也会调用）
 * ==================================================================== */

/* calc_safety_score — 综合多项指标计算驾驶安全分 (0~100)
 *   -25 温度超上限
 *   -15 温度低于下限
 *   -30 电机占空比>=80（超速）
 *   -20 蜂鸣器被屏蔽且温度超限（忽视报警）
 */
int calc_safety_score(void)
{
    int score = 100;

    if (g_temperature > g_temp_upLimit) {
        score -= 25; /* 过热 */
    }
    if (g_temperature < g_temp_doLimit) {
        score -= 15; /* 过冷 */
    }
    if (g_motor_duty >= 80) {
        score -= 30; /* 占空比过高，超速风险 */
    }
    /* 超限且蜂鸣器被屏蔽：驾驶员忽视了警报 */
    if (g_flag_buzzer_disable == 1 &&
        (g_temperature > g_temp_upLimit || g_temperature < g_temp_doLimit)) {
        score -= 20;
    }

    if (score < 0) score = 0;
    return score;
}

/* get_safety_grade — 将分值映射为等级 A/B/C/D */
static const char *get_safety_grade(int score)
{
    if (score >= 90) return "A";
    if (score >= 70) return "B";
    if (score >= 50) return "C";
    return "D";
}

/* ====================================================================
 * I2C & OLED 初始化
 * ==================================================================== */

/* i2c_oled_init — 配置I2C GPIO、初始化OLED、初始化AHT20 */
static void i2c_oled_init(void)
{
    /* 配置 GPIO13 → I2C0_SDA，上拉 */
    IoSetFunc(IOT_IO_NAME_GPIO_13, IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetPull(IOT_IO_NAME_GPIO_13, IOT_IO_PULL_UP);

    /* 配置 GPIO14 → I2C0_SCL，上拉 */
    IoSetFunc(IOT_IO_NAME_GPIO_14, IOT_IO_FUNC_GPIO_14_I2C0_SCL);
    IoSetPull(IOT_IO_NAME_GPIO_14, IOT_IO_PULL_UP);

    /* 初始化 I2C0，400kHz */
    IoTI2cInit(0, 400 * 1000);

    /* 初始化 SSD1306 OLED，显示启动提示 */
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 20);
    ssd1306_DrawString((char *)"Initializing...", Font_7x10, White);
    ssd1306_UpdateScreen();

    /* 初始化 AHT20 温湿度传感器（内部会操作I2C） */
    i2c_aht20_init();

    /* AHT20初始化后重新稳定I2C总线 */
    IoTI2cInit(0, 400 * 1000);
    usleep(30 * 1000); /* 等待30ms让总线稳定 */
}

/* ====================================================================
 * OLED 6屏显示函数
 * ==================================================================== */

/* 第1屏（0）：个人信息展示 */
static void oled_showInfo(void)
{
    ssd1306_Fill(Black);

    /* y=0：学号 */
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString((char *)"23001010613", Font_7x10, White);

    /* y=14：中文姓名（字库索引：0=李 1=松 2=伦） */
    ssd1306_SetCursor(0, 14);
    ssd1306_DrawChinese(0, Font_16x16, White); /* 李，光标自动右移16px */
    ssd1306_DrawChinese(1, Font_16x16, White); /* 松 */
    ssd1306_DrawChinese(2, Font_16x16, White); /* 伦 */

    /* y=34：英文名 */
    ssd1306_SetCursor(0, 34);
    ssd1306_DrawString((char *)"Li Songlun", Font_7x10, White);

    /* y=50：项目名 */
    ssd1306_SetCursor(0, 50);
    ssd1306_DrawString((char *)"CarMonitor", Font_7x10, White);

    ssd1306_UpdateScreen();
}

/* 第2屏（1）：温湿度实时数据 */
static void oled_showTempHumi(void)
{
    char buf[32];

    ssd1306_Fill(Black);

    /* y=0：温度（超限时附加提示） */
    if (g_temperature > g_temp_upLimit) {
        snprintf(buf, sizeof(buf), "temp:%.2f hi!!", g_temperature);
    } else if (g_temperature < g_temp_doLimit) {
        snprintf(buf, sizeof(buf), "temp:%.2f lo!!", g_temperature);
    } else {
        snprintf(buf, sizeof(buf), "temp:%.2f", g_temperature);
    }
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* y=16：湿度 */
    snprintf(buf, sizeof(buf), "humi:%.2f%%", g_humidity);
    ssd1306_SetCursor(0, 16);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* y=32：温度上限 */
    snprintf(buf, sizeof(buf), "temp_UpL:%.2f", g_temp_upLimit);
    ssd1306_SetCursor(0, 32);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* y=48：蜂鸣器状态 */
    int over = (g_temperature > g_temp_upLimit) ||
               (g_temperature < g_temp_doLimit);
    if (g_flag_buzzer_disable) {
        strncpy(buf, "buzzer X", sizeof(buf));   /* 已屏蔽 */
    } else if (over) {
        strncpy(buf, "buzzer on", sizeof(buf));  /* 超限报警中 */
    } else {
        strncpy(buf, "buzzer off", sizeof(buf)); /* 正常，静音 */
    }
    ssd1306_SetCursor(0, 48);
    ssd1306_DrawString(buf, Font_7x10, White);

    ssd1306_UpdateScreen();
}

/* 第3屏（2）：阈值调整（按键控制） */
static void oled_showThreshold(void)
{
    char buf[32];

    ssd1306_Fill(Black);

    /* y=0：上限（选中时加方括号） */
    if (g_flag_threshold_select == 0) {
        snprintf(buf, sizeof(buf), "[UpL:%.2f]", g_temp_upLimit);
    } else {
        snprintf(buf, sizeof(buf), " UpL:%.2f ", g_temp_upLimit);
    }
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* y=20：下限（选中时加方括号） */
    if (g_flag_threshold_select == 1) {
        snprintf(buf, sizeof(buf), "[DoL:%.2f]", g_temp_doLimit);
    } else {
        snprintf(buf, sizeof(buf), " DoL:%.2f ", g_temp_doLimit);
    }
    ssd1306_SetCursor(0, 20);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* y=40：按键提示 */
    ssd1306_SetCursor(0, 40);
    ssd1306_DrawString((char *)"K1+  K2-", Font_7x10, White);

    /* y=54：长按提示 */
    ssd1306_SetCursor(0, 54);
    ssd1306_DrawString((char *)"K1L:sel", Font_7x10, White);

    ssd1306_UpdateScreen();
}

/* ====================================================================
 * 方案A：第4屏（3）—— 半圆形速度仪表盘
 *
 * 布局（128×64）：
 *   y=0~38   上半部分：半圆弧 + 刻度线 + 指针 + 标签
 *   y=44~63  下半部分：数字信息（速度/方向/占空比/模式）
 *
 * 仪表盘参数：
 *   圆心 (cx=64, cy=38)，半径 r=30px
 *   角度0°=右端(100%) ↔ 角度180°=左端(0%)
 *   指针角度 = 180 - duty * 180 / 100
 *
 * 三角函数用整数查找表实现，避免依赖 libm（Hi3861 scons 不链接 -lm）：
 *   sin_lut[d] = sin(d°) × 1000，d = 0..180
 * ==================================================================== */

/* sin_lut[d] = sin(d°) × 1000，d = 0..180 */
static const int16_t sin_lut[181] = {
       0,  17,  35,  52,  70,  87, 105, 122, 139, 156,  /* 0-9   */
     174, 191, 208, 225, 242, 259, 276, 292, 309, 326,  /* 10-19 */
     342, 358, 375, 391, 407, 423, 438, 454, 469, 485,  /* 20-29 */
     500, 515, 530, 545, 559, 574, 588, 602, 616, 629,  /* 30-39 */
     643, 656, 669, 682, 695, 707, 719, 731, 743, 755,  /* 40-49 */
     766, 777, 788, 799, 809, 819, 829, 839, 848, 857,  /* 50-59 */
     866, 875, 883, 891, 899, 906, 914, 921, 927, 934,  /* 60-69 */
     940, 946, 951, 956, 961, 966, 970, 974, 978, 982,  /* 70-79 */
     985, 988, 990, 993, 995, 996, 998, 999, 999,1000,  /* 80-89 */
    1000,                                                 /* 90    */
    1000, 999, 999, 998, 996, 995, 993, 990, 988, 985,  /* 91-100*/
     982, 978, 974, 970, 966, 961, 956, 951, 946, 940,  /* 101-110*/
     934, 927, 921, 914, 906, 899, 891, 883, 875, 866,  /* 111-120*/
     857, 848, 839, 829, 819, 809, 799, 788, 777, 766,  /* 121-130*/
     755, 743, 731, 719, 707, 695, 682, 669, 656, 643,  /* 131-140*/
     629, 616, 602, 588, 574, 559, 545, 530, 515, 500,  /* 141-150*/
     485, 469, 454, 438, 423, 407, 391, 375, 358, 342,  /* 151-160*/
     326, 309, 292, 276, 259, 242, 225, 208, 191, 174,  /* 161-170*/
     156, 139, 122, 105,  87,  70,  52,  35,  17,   0   /* 171-180*/
};

/* cos(d°) × 1000：利用恒等式 cos(d) = sin(90-d) */
static int my_cos_1000(int d)
{
    if (d <= 90) return (int)sin_lut[90 - d];
    return -(int)sin_lut[d - 90];
}

static void oled_showMotor(void)
{
    char buf[32];

    ssd1306_Fill(Black);

    /* ====== 上半区域：半圆仪表盘（圆心(64,32)，半径24） ====== */

    /* ——— 步骤1：绘制半圆弧（angle=0°~180°，步长2°） ——— */
    for (int angle = 0; angle <= 180; angle += 2) {
        int x = 64 + 24 * my_cos_1000(angle) / 1000;
        int y = 32 - 24 * (int)sin_lut[angle]  / 1000;
        ssd1306_DrawPixel((uint8_t)x, (uint8_t)y, White);
    }

    /* ——— 步骤2：危险区（速度>=2.4m/s 对应 angle=0°~36°，三层加粗） ——— */
    for (int angle = 0; angle <= 36; angle += 2) {
        for (int r = 22; r <= 24; r++) {
            int x = 64 + r * my_cos_1000(angle) / 1000;
            int y = 32 - r * (int)sin_lut[angle]  / 1000;
            ssd1306_DrawPixel((uint8_t)x, (uint8_t)y, White);
        }
    }

    /* ——— 步骤3：5个主刻度线（角度180/135/90/45/0°，半径20→24） ——— */
    {
        int tick_angles[5] = {180, 135, 90, 45, 0};
        for (int i = 0; i < 5; i++) {
            int a  = tick_angles[i];
            int x1 = 64 + 20 * my_cos_1000(a) / 1000;
            int y1 = 32 - 20 * (int)sin_lut[a] / 1000;
            int x2 = 64 + 24 * my_cos_1000(a) / 1000;
            int y2 = 32 - 24 * (int)sin_lut[a] / 1000;
            ssd1306_DrawLine((uint8_t)x1, (uint8_t)y1,
                             (uint8_t)x2, (uint8_t)y2, White);
        }
    }

    /* ——— 步骤4：指针（圆心→r=20，角度由速度决定） ——— */
    {
        /* 速度量程 0~1.0 m/s，超过1.0则钳制到最右端（0°） */
        float speed_clamped = g_speed;
        if (speed_clamped > 1.0f) speed_clamped = 1.0f;
        int angle = (int)(180.0f - speed_clamped / 1.0f * 180.0f);
        int x_end = 64 + 20 * my_cos_1000(angle) / 1000;
        int y_end = 32 - 20 * (int)sin_lut[angle] / 1000;
        ssd1306_DrawLine(64, 32, (uint8_t)x_end, (uint8_t)y_end, White);
    }

    /* ——— 步骤5：圆心点（3×3小方块） ——— */
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            ssd1306_DrawPixel((uint8_t)(64 + dx), (uint8_t)(32 + dy), White);
        }
    }

    /* ——— 步骤6：刻度标签（适配新圆心位置） ——— */
    ssd1306_SetCursor(2, 24);
    ssd1306_DrawString((char *)"0", Font_7x10, White);

    ssd1306_SetCursor(56, 4);
    ssd1306_DrawString((char *)"0.5", Font_7x10, White);

    ssd1306_SetCursor(106, 24);
    ssd1306_DrawString((char *)"1.0", Font_7x10, White);

    /* ——— 分隔线（y=36，横跨全宽，将表盘与数字区隔开） ——— */
    ssd1306_DrawLine(0, 36, 127, 36, White);

    /* ====== 下半左侧（x=0~62）：速度数字信息 ====== */

    /* y=40：当前速度，仅在完全静止时显示"--" */
    if (g_speed < 0.01f && g_motor_duty == 0) {
        strncpy(buf, "--.- m/s", sizeof(buf));
    } else {
        snprintf(buf, sizeof(buf), "%.2f m/s", g_speed);
    }
    ssd1306_SetCursor(0, 40);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* y=50：运行模式 + 方向（有速度但duty==0时仍显示实际方向） */
    {
        const char *mode;
        const char *dir_str;
        if (g_motor_duty == 0 && g_speed < 0.01f) {
            /* 完全静止 */
            mode    = "STOP";
            dir_str = "--";
        } else if (g_motor_duty >= 80) {
            mode    = "SPRT";
            dir_str = (g_motor_dir == MOTOR_DIR_CW) ? "FWD" : "BWD";
        } else if (g_motor_duty >= 50) {
            mode    = "NRM";
            dir_str = (g_motor_dir == MOTOR_DIR_CW) ? "FWD" : "BWD";
        } else if (g_motor_duty > 0) {
            mode    = "ECO";
            dir_str = (g_motor_dir == MOTOR_DIR_CW) ? "FWD" : "BWD";
        } else {
            /* duty==0 但有速度（手动旋转） */
            mode    = "STOP";
            dir_str = (g_motor_dir == MOTOR_DIR_CW) ? "FWD" : "BWD";
        }
        snprintf(buf, sizeof(buf), "%s %s", mode, dir_str);
    }
    ssd1306_SetCursor(0, 50);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* ====== 下半右侧（x=65~127）：速度历史折线图 ====== */

    /* 左侧竖线作为折线图边框 */
    ssd1306_DrawLine(65, 39, 65, 54, White);

    /* 绘制折线：24个点，间距2px，最新数据在最右侧 */
    {
        int prev_x = 0, prev_y = 0;
        for (int i = 0; i < SPEED_HISTORY_SIZE; i++) {
            /* 从环形缓冲区按时间先后顺序（旧→新）取数据 */
            int data_idx = (g_speed_history_idx - SPEED_HISTORY_SIZE + i
                            + SPEED_HISTORY_SIZE) % SPEED_HISTORY_SIZE;
            int x = 65 + i * 2;
            float sv = g_speed_history[data_idx];
            if (sv > 1.0f) sv = 1.0f;
            /* y轴：速度0对应y=54（底部），速度1.0对应y=39（顶部） */
            int y = 54 - (int)(sv / 1.0f * 15.0f);
            if (y < 39) y = 39;
            if (y > 54) y = 54;
            /* 与前一点连线（i==0时无前驱，仅记录坐标） */
            if (i > 0) {
                ssd1306_DrawLine((uint8_t)prev_x, (uint8_t)prev_y,
                                 (uint8_t)x,      (uint8_t)y, White);
            }
            prev_x = x;
            prev_y = y;
        }
    }

    ssd1306_UpdateScreen();
}

/* ====================================================================
 * 方案B：第5屏（4）—— 驾驶安全评分系统
 *
 * 布局：
 *   y=0   标题 "Safety Score"
 *   y=14  进度条（外框+填充）+ 分值
 *   y=28  温度状态 + 速度状态
 *   y=40  蜂鸣器状态 + 等级
 *   y=52  等级对应提示语
 * ==================================================================== */
static void oled_showSafety(void)
{
    char buf[32];

    ssd1306_Fill(Black);

    /* 计算当前分值和等级 */
    int score = calc_safety_score();
    const char *grade = get_safety_grade(score);

    /* ——— y=0：标题 ——— */
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString((char *)"Safety Score", Font_7x10, White);

    /* ——— y=14~22：进度条 ——— */
    /* 外框上下两条线（全宽） */
    ssd1306_DrawLine(0, 14, 127, 14, White);
    ssd1306_DrawLine(0, 22, 127, 22, White);

    /* 填充部分（x=1 ~ bar_fill，留右侧空间给数字）
     * bar_fill ∈ [0, 96]，右侧 x=100~127 留给分值文字 */
    int bar_fill = score * 96 / 100;
    for (int x = 1; x <= bar_fill; x++) {
        for (int y = 15; y <= 21; y++) {
            ssd1306_DrawPixel((uint8_t)x, (uint8_t)y, White);
        }
    }

    /* 分值数字（右侧空白区，x=100，背景为黑，清晰可读） */
    snprintf(buf, sizeof(buf), "%d", score);
    ssd1306_SetCursor(100, 14);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* ——— y=28：温度状态 + 速度状态 ——— */
    {
        const char *temp_st;
        if (g_temperature > g_temp_upLimit)      temp_st = "HIGH";
        else if (g_temperature < g_temp_doLimit) temp_st = "LOW";
        else                                      temp_st = "OK";
        const char *spd_st = (g_motor_duty >= 80) ? "WARN" : "OK";
        snprintf(buf, sizeof(buf), "Temp:%s Spd:%s", temp_st, spd_st);
    }
    ssd1306_SetCursor(0, 28);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* ——— y=40：蜂鸣器状态 + 等级 ——— */
    snprintf(buf, sizeof(buf), "Buz:%s Grade:%s",
             g_flag_buzzer_disable ? "OFF" : "ON", grade);
    ssd1306_SetCursor(0, 40);
    ssd1306_DrawString(buf, Font_7x10, White);

    /* ——— y=52：等级提示语 ——— */
    {
        const char *msg;
        switch (grade[0]) {
            case 'A': msg = "Drive Safe!"; break;
            case 'B': msg = "Stay Alert";  break;
            case 'C': msg = "Slow Down!";  break;
            default:  msg = "DANGER!!!";   break;
        }
        ssd1306_SetCursor(0, 52);
        ssd1306_DrawString((char *)msg, Font_7x10, White);
    }

    ssd1306_UpdateScreen();
}

/* 第6屏（5）：华为云下发命令记录 */
static void oled_showCloudCmd(void)
{
    ssd1306_Fill(Black);

    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString((char *)"Cloud CMD:", Font_7x10, White);

    ssd1306_SetCursor(0, 16);
    ssd1306_DrawString(g_cloud_cmd_line1, Font_7x10, White);

    ssd1306_SetCursor(0, 32);
    ssd1306_DrawString(g_cloud_cmd_line2, Font_7x10, White);

    ssd1306_SetCursor(0, 48);
    ssd1306_DrawString(g_cloud_cmd_line3, Font_7x10, White);

    ssd1306_UpdateScreen();
}

/* refresh_screen — 根据当前屏号刷新显示（6屏） */
static void refresh_screen(void)
{
    switch (g_flag_show) {
        case 0: oled_showInfo();      break; /* 个人信息 */
        case 1: oled_showTempHumi();  break; /* 温湿度 */
        case 2: oled_showThreshold(); break; /* 阈值调整 */
        case 3: oled_showMotor();     break; /* 仪表盘（方案A） */
        case 4: oled_showSafety();    break; /* 安全评分（方案B） */
        case 5: oled_showCloudCmd();  break; /* 云命令记录 */
        default: break;
    }
}

/* ====================================================================
 * 线程1：TempHumiTask — 每500ms采集温湿度并上报华为云
 * ==================================================================== */
static void TempHumiTask(void)
{
    while (1) {
        osDelay(50); /* 500ms（50 ticks × 10ms/tick） */

        /* 采集 AHT20 温湿度 */
        if (aht20_getDat(&g_temperature, &g_humidity) != 0) {
            printf("aht20_getDat failed\n");
            continue;
        }
        printf("23001010613 temp=%.2f humi=%.2f score=%d\n",
               g_temperature, g_humidity, calc_safety_score());

        /* 判断超限，控制蜂鸣器和LED */
        int over = (g_temperature > g_temp_upLimit) ||
                   (g_temperature < g_temp_doLimit);
        if (over && !g_flag_buzzer_disable) {
            /* 超限且未屏蔽：蜂鸣器报警 + LED亮 */
            buzzer_pwm_start1(50, 3000);
            led_on();
        } else if (!over) {
            /* 温度正常：静音 + LED灭（无论是否屏蔽） */
            buzzer_pwm_stop();
            led_off();
        }
        /* 若 over && g_flag_buzzer_disable：保持当前状态（不重新触发） */

        /* 上报华为云（含安全评分） */
        deal_report_msg();
    }
}

/* ====================================================================
 * 线程2：handler_Key_Task — 每50ms轮询按键
 * ==================================================================== */
static void handler_Key_Task(void)
{
    while (1) {
        osDelay(5); /* 50ms */

        /* ——— KEY0：切换屏幕（短按）/ 屏蔽蜂鸣器（长按） ——— */
        int key0 = readAdCKey0();
        if (key0 == KEY_PRESS_kernel) {
            /* 短按：屏幕循环切换 0→1→2→3→4→5→0（6屏） */
            g_flag_show = (g_flag_show + 1) % 6;
            refresh_screen();
        } else if (key0 == KEY_PRESSLONG_kernel) {
            /* 长按：翻转蜂鸣器屏蔽标志 */
            g_flag_buzzer_disable = !g_flag_buzzer_disable;
            if (g_flag_buzzer_disable) {
                /* 屏蔽后立即停止蜂鸣器和LED */
                buzzer_pwm_stop();
                led_off();
            }
            refresh_screen();
        }

        /* ——— KEY1 & KEY2：按当前屏号分流 ——— */
        int key1 = readAdCKey1();
        int key2 = readAdCKey2();

        if (g_flag_show == 2) {
            /* 第3屏（阈值调整） */
            if (key1 == KEY1_PRESS) {
                /* 短按：选中的阈值 +1 */
                if (g_flag_threshold_select == 0) {
                    g_temp_upLimit += 1.0f;
                } else {
                    g_temp_doLimit += 1.0f;
                    /* 确保下限 < 上限 */
                    if (g_temp_doLimit >= g_temp_upLimit) {
                        g_temp_doLimit = g_temp_upLimit - 1.0f;
                    }
                }
                refresh_screen();
            } else if (key1 == KEY1_PRESSLONG) {
                /* 长按：切换选中项（上限↔下限） */
                g_flag_threshold_select = (g_flag_threshold_select + 1) % 2;
                refresh_screen();
            }

            if (key2 == KEY2_PRESS) {
                /* 短按：选中的阈值 -1 */
                if (g_flag_threshold_select == 0) {
                    g_temp_upLimit -= 1.0f;
                    /* 确保上限 > 下限 */
                    if (g_temp_upLimit <= g_temp_doLimit) {
                        g_temp_upLimit = g_temp_doLimit + 1.0f;
                    }
                } else {
                    g_temp_doLimit -= 1.0f;
                }
                refresh_screen();
            }

        } else if (g_flag_show == 3) {
            /* 第4屏（电机仪表盘控制） */
            if (key1 == KEY1_PRESS) {
                /* 短按：切换方向 CW↔CCW */
                /* g_motor_dir 是 int8_t（定义于 pwm_motor.c），枚举赋值需显式转换 */
                g_motor_dir = (int8_t)((g_motor_dir == MOTOR_DIR_CW)
                              ? MOTOR_DIR_CCW : MOTOR_DIR_CW);
                /* 若电机正在运行，立即切换方向 */
                if (g_motor_duty != 0) {
                    pwm_motor_start((MotorDir)g_motor_dir, g_motor_duty);
                }
                refresh_screen();
            }

            if (key2 == KEY2_PRESS) {
                /* 短按：循环切换占空比 0→20→50→80→0 */
                if (g_motor_duty == 0)       g_motor_duty = 20;
                else if (g_motor_duty == 20) g_motor_duty = 50;
                else if (g_motor_duty == 50) g_motor_duty = 80;
                else                         g_motor_duty = 0;

                if (g_motor_duty == 0) {
                    pwm_motor_stop();
                } else {
                    pwm_motor_start((MotorDir)g_motor_dir, g_motor_duty);
                }
                refresh_screen();
            } else if (key2 == KEY2_PRESSLONG) {
                /* 长按：强制停止电机 */
                pwm_motor_stop();
                g_motor_duty = 0;
                refresh_screen();
            }
        }
    }
}

/* ====================================================================
 * 线程3：handler_oled_Task — 每500ms刷新当前屏
 * ==================================================================== */
static void handler_oled_Task(void)
{
    while (1) {
        osDelay(50); /* 500ms */
        /* 每次OLED刷新时记录一次速度，与显示周期同步，避免折线图阶梯状 */
        g_speed_history[g_speed_history_idx] = g_speed;
        g_speed_history_idx = (g_speed_history_idx + 1) % SPEED_HISTORY_SIZE;
        refresh_screen();
    }
}

/* ====================================================================
 * 线程4：handler_encoder_Task — 每100ms计算一次车速
 * ==================================================================== */
static void handler_encoder_Task(void)
{
    while (1) {
        osDelay(10); /* 100ms */

        /* 读取并清零增量脉冲数（简单原子操作，单核RISC-V可接受） */
        int delta = g_encoder_pulse_delta;
        g_encoder_pulse_delta = 0;

        /* 速度计算 + 5点滑动平均平滑处理 */
        static float speed_buf[5]  = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; /* 滑动窗口缓冲区 */
        static int   speed_buf_idx = 0;                                /* 滑动窗口写指针 */

        /* 异常值过滤：超过300脉冲/100ms视为干扰，跳过本次更新保持上次速度 */
        if (delta > 300) {
            g_encoder_pulse_delta = 0;
            continue;
        }

        if (delta == 0) {
            /* 静止时直接清零整个窗口，不参与滑动平均，避免历史值拖慢归零 */
            speed_buf[0] = speed_buf[1] = speed_buf[2] =
            speed_buf[3] = speed_buf[4] = 0.0f;
            speed_buf_idx = 0;
            g_speed = 0.0f;
        } else {
            /* 速度 = 车轮周长(0.16m) × 脉冲数 / 256 / 计算周期(0.1s) */
            float raw_speed = 0.16f * (float)delta / 256.0f / 0.1f;
            /* 写入滑动窗口，取5次平均，进一步平滑速度曲线 */
            speed_buf[speed_buf_idx] = raw_speed;
            speed_buf_idx = (speed_buf_idx + 1) % 5;
            g_speed = (speed_buf[0] + speed_buf[1] + speed_buf[2] +
                       speed_buf[3] + speed_buf[4]) / 5.0f;
        }

        printf("23001010613 encoder: delta=%d speed=%.2f m/s dir=%s\n",
               delta, g_speed, g_encoder_dir == 0 ? "CW" : "CCW");
    }
}

/* ====================================================================
 * 设备外设初始化
 * ==================================================================== */
static void device_init(void)
{
    led_init();              /* LED：GPIO9，普通GPIO输出模式 */
    buzzer_pwm_init();       /* 蜂鸣器：GPIO9，PWM0模式 */
    buzzer_pwm_stop();       /* 上电默认关闭蜂鸣器 */
    pwm_motor_init(MOTOR_DIR_CW); /* 电机：GPIO1(IN1) + GPIO2(IN2)，默认CW方向 */
    i2c_oled_init();         /* I2C0 + OLED + AHT20 */
    encoder_init();          /* 编码器：GPIO7（A相中断）+ GPIO6（B相输入） */
}

/* ====================================================================
 * mainEntry — 系统入口（SYS_RUN 注册）
 * ==================================================================== */
static void mainEntry(void)
{
    printf("23001010613 李松伦 mqtt_hwcloud_experiment start!\n");

    /* 初始化所有外设 */
    device_init();

    /* 连接 WiFi */
    WifiTask();

    /* 连接华为云 MQTT，启动命令接收线程 */
    huawei_cloud_mqtt_init();

    /* 创建任务属性模板 */
    osThreadAttr_t attr;
    attr.attr_bits = 0U;
    attr.cb_mem    = NULL;
    attr.cb_size   = 0U;
    attr.stack_mem = NULL;

    /* 线程1：温湿度采集 + 云上报，栈4096 */
    attr.name       = "TempHumiTask";
    attr.stack_size = 1024 * 4;
    attr.priority   = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)TempHumiTask, NULL, &attr) == NULL) {
        printf("create TempHumiTask failed\n");
    }

    /* 线程2：按键处理，栈2048 */
    attr.name       = "KeyTask";
    attr.stack_size = 1024 * 2;
    attr.priority   = osPriorityNormal1;
    if (osThreadNew((osThreadFunc_t)handler_Key_Task, NULL, &attr) == NULL) {
        printf("create KeyTask failed\n");
    }

    /* 线程3：OLED定时刷新，栈4096 */
    attr.name       = "OledTask";
    attr.stack_size = 1024 * 4;
    attr.priority   = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)handler_oled_Task, NULL, &attr) == NULL) {
        printf("create OledTask failed\n");
    }

    /* 线程4：编码器测速，栈2048 */
    attr.name       = "EncoderTask";
    attr.stack_size = 1024 * 2;
    attr.priority   = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)handler_encoder_Task, NULL, &attr) == NULL) {
        printf("create EncoderTask failed\n");
    }
}

/* 注册为系统启动函数 */
SYS_RUN(mainEntry);
