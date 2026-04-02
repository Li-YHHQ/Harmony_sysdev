#ifndef ADCKEY_H
#define ADCKEY_H
 
#define KEY_ADC_CHANNEL   HI_ADC_CHANNEL_2 //GPIO5:ADC_CHANNEL_2

/* 定义按键按下和未按下变量*/

//以下的宏为采集到的按键值
#define KEY_IDLE        0

#define KEY_PRESS_kernel      1
#define KEY_PRESSLONG_kernel  2

#define KEY1_PRESS      3
#define KEY1_PRESSLONG  4

#define KEY2_PRESS      5
#define KEY2_PRESSLONG  6

//以下为按键状态，分别为抬起和按下
#define RELEASED        9
#define PRESSED         10



int readAdCKey0(void);

// oled屏s1按键，返回值为：0-无按键，3-短按，4-长按
int readAdCKey1(void);

// oled屏s2按键，返回值为：0-无按键，5-短按，6-长按
int readAdCKey2(void);

#endif