#include "key.h"
#include "iot_gpio_ex.h"
#include "cmsis_os2.h"

void key_init(void)
{
    IoTGpioInit(KEY_GPIO);
    IoSetFunc(KEY_GPIO, IOT_IO_FUNC_GPIO_5_GPIO);
    IoTGpioSetDir(KEY_GPIO, IOT_GPIO_DIR_IN);
    IoSetPull(KEY_GPIO, IOT_IO_PULL_UP);
}

int readKey(void)
{
    static int is_pressed = RELEASED;
    static int keyPressTime = 0;
    IotGpioValue val;

    IoTGpioGetInputVal(KEY_GPIO, &val);

    if ((val == 0) && (is_pressed == RELEASED)) {
        osDelay(1);
        IoTGpioGetInputVal(KEY_GPIO, &val);
        if (val == 0) {
            is_pressed = PRESSED;
            keyPressTime = osKernelGetTickCount();
        }
    } else if ((val == 1) && (is_pressed == PRESSED)) {
        is_pressed = RELEASED;
        if ((osKernelGetTickCount() - keyPressTime) >= 300) {
            return KEY_PRESSLONG_kernel;
        }
        return KEY_PRESS_kernel;
    }
    return KEY_NONE;
}
