/*
 * led.h — GPIO9 LED 控制接口
 * GPIO9 与蜂鸣器共用同一引脚：
 *   GPIO模式（普通输出）→ 控制LED
 *   PWM0模式            → 控制蜂鸣器
 * 低电平亮（VALUE0），高电平灭（VALUE1）
 */

#ifndef LED_H
#define LED_H

/* LED初始化：将GPIO9设为普通GPIO输出模式 */
void led_init(void);

/* 点亮LED（GPIO9输出低电平） */
void led_on(void);

/* 熄灭LED（GPIO9输出高电平） */
void led_off(void);

#endif /* LED_H */
