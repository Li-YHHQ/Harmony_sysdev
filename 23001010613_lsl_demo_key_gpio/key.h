#ifndef KEY_H
#define KEY_H
#include "iot_gpio.h"

#define KEY_GPIO IOT_IO_NAME_GPIO_5

#define KEY_NONE             0
#define KEY_PRESS_kernel     1
#define KEY_PRESSLONG_kernel 2

#define RELEASED 9
#define PRESSED  10

void key_init(void);
int readKey(void);

#endif
