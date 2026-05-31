/* encoder.h — 编码器驱动接口
 * A相：GPIO_7（上升沿中断计脉冲）
 * B相：GPIO_6（读取电平判断方向）
 */
#ifndef ENCODER_H
#define ENCODER_H

/* 初始化编码器：配置GPIO并注册A相中断 */
void encoder_init(void);

#endif /* ENCODER_H */
