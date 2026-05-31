// AHT20数字温湿度传感器驱动接口文件

// 定义条件编译宏，防止头文件的重复包含和编译
#ifndef AHT20_H
#define AHT20_H

#include <stdint.h>     // 定义了几种扩展的整数类型和宏

// 声明接口函数

// 接口函数：aht20初始化
void i2c_aht20_init();

// 接口函数：获取温湿度数据。
//返回值：失败-1，成功0
int aht20_getDat(float *temp, float *humi);

// 条件编译结束
#endif  // AHT20_H
