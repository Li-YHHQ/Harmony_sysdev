#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

#include <string.h>
#include "hi_i2c.h"

/* 使用 I2C 方式 */
#define SSD1306_USE_I2C

/* I2C 端口号与设备地址（7位） */
#define SSD1306_I2C_PORT   0
#define SSD1306_I2C_ADDR   0x3C

/* 屏幕分辨率 */
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64

/* 底层写命令宏：发送控制字节 0x00 + 命令字节 */
#define SSD1306_I2C_WriteCmd(cmd)                                  \
    do {                                                           \
        uint8_t _buf[2] = {0x00, (uint8_t)(cmd)};                 \
        hi_i2c_data _d = {_buf, 2, 0, 0};                         \
        hi_i2c_write(SSD1306_I2C_PORT, SSD1306_I2C_ADDR, &_d);   \
    } while (0)

/* 底层写数据宏：在数据前加控制字节 0x40 后发送 */
#define SSD1306_I2C_WriteData(buf, len)                            \
    do {                                                           \
        uint8_t _tmp[(len) + 1];                                   \
        _tmp[0] = 0x40;                                            \
        memcpy(_tmp + 1, (buf), (len));                            \
        hi_i2c_data _d = {_tmp, (len) + 1, 0, 0};                 \
        hi_i2c_write(SSD1306_I2C_PORT, SSD1306_I2C_ADDR, &_d);   \
    } while (0)

/* 包含所需字体 */
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_16x26

#endif /* __SSD1306_CONF_H__ */
