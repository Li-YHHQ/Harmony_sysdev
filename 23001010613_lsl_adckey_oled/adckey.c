
#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "hi_io.h"      // 海思 Pegasus SDK：IoT硬件设备操作接口-IO
#include "hi_adc.h"     // 海思 Pegasus SDK：IoT硬件设备操作接口-ADC
#include "adckey.h"

// 补全头文件


// 核心板按键，短按、长按读取函数
int readAdCKey0(void){
    static uint8_t is_pressed= RELEASED;
    static int keyPressTime = 0;                    //识别长按
    unsigned short data = 0;// 用于存放ADC2通道的值

    // 读取ADC2通道的值
    if (hi_adc_read(KEY_ADC_CHANNEL, &data,  HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) != HI_ERR_SUCCESS){
        
        return -1;
    }
    
    // 如果ADC2通道的值大于等于5，小于228”
    if ( (data >= 5 && data < 228)  && (is_pressed==RELEASED) )    //按键按下一刻
    {
        //printf("S1 Button! ADC_VALUE = %d\n", data);
        is_pressed=KEY_PRESS_kernel;
        keyPressTime=osKernelGetTickCount();
            
    }else if( (data>682) && (is_pressed==KEY_PRESS_kernel) ){   //按键抬起时刻,并且返回按键值：长按或者短按。
        is_pressed=RELEASED;
        if ((osKernelGetTickCount() - keyPressTime) >= 200) {  //按键松开时判断是长按还是短按。
            return KEY_PRESSLONG_kernel;
        }else{
            return KEY_PRESS_kernel;
        }  

    }
  
}

// oled屏s1按键，短按返回3，长按返回4，无动作返回0，adc值范围：228-455
int readAdCKey1(void)
{
    static uint8_t is_pressed = RELEASED;
    static int keyPressTime = 0;
    unsigned short data = 0;

    if (hi_adc_read(KEY_ADC_CHANNEL, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) != HI_ERR_SUCCESS) {
        return -1;
    }

    if ((data >= 228 && data < 455) && (is_pressed == RELEASED)) {
        is_pressed = KEY1_PRESS;
        keyPressTime = osKernelGetTickCount();
    } else if ((data > 682) && (is_pressed == KEY1_PRESS)) {
        is_pressed = RELEASED;
        if ((osKernelGetTickCount() - keyPressTime) >= 200) {
            return KEY1_PRESSLONG;
        } else {
            return KEY1_PRESS;
        }
    }

    return KEY_IDLE;
}

// oled屏s2按键，短按返回5，长按返回6，无动作返回0，adc值范围：455-682
int readAdCKey2(void)
{
    static uint8_t is_pressed = RELEASED;
    static int keyPressTime = 0;
    unsigned short data = 0;

    if (hi_adc_read(KEY_ADC_CHANNEL, &data, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) != HI_ERR_SUCCESS) {
        return -1;
    }

    if ((data >= 455 && data < 682) && (is_pressed == RELEASED)) {
        is_pressed = KEY2_PRESS;
        keyPressTime = osKernelGetTickCount();
    } else if ((data > 682) && (is_pressed == KEY2_PRESS)) {
        is_pressed = RELEASED;
        if ((osKernelGetTickCount() - keyPressTime) >= 200) {
            return KEY2_PRESSLONG;
        } else {
            return KEY2_PRESS;
        }
    }

    return KEY_IDLE;
}
