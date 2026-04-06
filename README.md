# 鸿蒙系统开发课程实验

> 基于 OpenHarmony 3.0.5 LTS | Hi3861 开发板 | C 语言

---

## 环境信息

| 项目 | 说明 |
|------|------|
| 操作系统 | OpenHarmony 3.0.5 LTS |
| 开发板 | Hi3861（海思 HiSilicon IoT 芯片） |
| 内核 | LiteOS-M |
| 构建系统 | GN（Google Ninja） |
| 开发语言 | C |

---

## 已完成项目

### 1. Hello World — `lsl613_helloworld`

**目录**：`lsl613_helloworld/`

**实验目的**：熟悉 OpenHarmony 工程结构与 `SYS_RUN()` 系统初始化宏的使用，完成第一个在开发板上运行的程序。

**实验内容**：
- 通过串口输出学号及 `Hello, World` 字符串
- 使用 `ohos_init.h` 提供的 `SYS_RUN()` 宏注册系统启动任务

**关键文件**：

| 文件 | 说明 |
|------|------|
| `lsl613_helloworld.c` | 主程序，串口打印输出 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
void HelloWorld(void)
{
    printf("\nlsl23001010613\n");
    printf("Hello, World\n");
}

SYS_RUN(HelloWorld);
```

---

### 2. LED 灯闪烁控制 — `lsl613_ledcontrol`

**目录**：`lsl613_ledcontrol/`

**实验目的**：学习 OpenHarmony IoT 硬件 GPIO 接口的使用，通过控制 GPIO 引脚电平实现 LED 灯周期性闪烁。

**实验内容**：
- 初始化 GPIO_9 引脚并配置为输出模式
- 循环切换引脚高低电平，每次延时 300 ms，实现 LED 闪烁效果
- 使用 CMSIS-RTOS2 的 `osDelay()` 进行任务延时

**关键文件**：

| 文件 | 说明 |
|------|------|
| `lsl613_ledcontrol.c` | 主程序，GPIO 控制逻辑 |
| `iot_gpio_ex.h` | GPIO 引脚功能枚举及接口声明 |
| `hal_iot_gpio_ex.c` | GPIO 扩展功能硬件抽象层实现 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
#define DELAY_MS 300

void led_control(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_9);
    IoSetFunc(IOT_IO_NAME_GPIO_9, IOT_IO_FUNC_GPIO_9_GPIO);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_9, IOT_GPIO_DIR_OUT);
    while (1) {
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_9, IOT_GPIO_VALUE1); // 点亮
        osDelay(DELAY_MS);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_9, IOT_GPIO_VALUE0); // 熄灭
        osDelay(DELAY_MS);
    }
}

SYS_RUN(led_control);
```

**硬件连接**：LED 正极连接 GPIO_9 引脚，负极接地（GND）。

---

### 3. 三灯同步闪烁 — `23001010613_lsl_LiSonglun_blueled_blink`

**目录**：`23001010613_lsl_LiSonglun_blueled_blink/`

**实验目的**：在多路 GPIO 控制基础上，实现三个 LED 灯同步闪烁。

**实验内容**：
- 同时初始化 GPIO_10（红）、GPIO_11（绿）、GPIO_12（黄）三路引脚为输出模式
- 三灯同步点亮 / 同步熄灭，延时 1 s（`osDelay(100)`，单位 10 ms）

**关键文件**：

| 文件 | 说明 |
|------|------|
| `23001010613_lsl_LiSonglun_blueled_blink.c` | 主程序，三路 GPIO 同步控制 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
#define DELAY_MS 100  // 100*10ms = 1s

while (1) {
    IoTGpioSetOutputVal(10, IOT_GPIO_VALUE1); // 三灯同时点亮
    IoTGpioSetOutputVal(11, IOT_GPIO_VALUE1);
    IoTGpioSetOutputVal(12, IOT_GPIO_VALUE1);
    osDelay(DELAY_MS);
    IoTGpioSetOutputVal(10, IOT_GPIO_VALUE0); // 三灯同时熄灭
    IoTGpioSetOutputVal(11, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(12, IOT_GPIO_VALUE0);
    osDelay(DELAY_MS);
}
```

**硬件连接**：红灯接 GPIO_10，绿灯接 GPIO_11，黄灯接 GPIO_12，负极均接 GND。

---

### 4. 交通灯模拟 — `23001010613_lsl_LiSonglun_ledblink`

**目录**：`23001010613_lsl_LiSonglun_ledblink/`

**实验目的**：综合运用多路 GPIO 控制，模拟交通灯红→绿→黄循环切换逻辑。

**实验内容**：
- 初始化 GPIO_10（红）、GPIO_11（绿）、GPIO_12（黄）三路引脚为输出模式
- 依次循环：红灯亮 3 s → 绿灯亮 3 s → 黄灯亮 3 s
- 每次切换时保证其余两灯熄灭，实现互斥点亮

**关键文件**：

| 文件 | 说明 |
|------|------|
| `23001010613_lsl_LiSonglun_ledblink.c` | 主程序，交通灯顺序切换逻辑 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
#define DELAY_MS 300  // 300*10ms = 3s

while (1) {
    IoTGpioSetOutputVal(10, IOT_GPIO_VALUE1); // 红亮
    IoTGpioSetOutputVal(11, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(12, IOT_GPIO_VALUE0);
    osDelay(DELAY_MS);

    IoTGpioSetOutputVal(10, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(11, IOT_GPIO_VALUE1); // 绿亮
    IoTGpioSetOutputVal(12, IOT_GPIO_VALUE0);
    osDelay(DELAY_MS);

    IoTGpioSetOutputVal(10, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(11, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(12, IOT_GPIO_VALUE1); // 黄亮
    osDelay(DELAY_MS);
}
```

**硬件连接**：红灯接 GPIO_10，绿灯接 GPIO_11，黄灯接 GPIO_12，负极均接 GND。

---

### 5. 行人过街灯 — `23001010613_lsl_LiSonglun_pedestrian`

**目录**：`23001010613_lsl_LiSonglun_pedestrian/`

**实验目的**：模拟真实行人过街灯时序，掌握多阶段 GPIO 控制与绿灯快闪提示的实现方法。

**实验内容**：行人过街灯按以下四阶段循环运行：

| 阶段 | 状态 | 时长 | 说明 |
|------|------|------|------|
| 1 | 绿灯长亮 | 3 s | 行人通行 |
| 2 | 绿灯快闪 3 次 | 约 0.6 s | 提示即将禁行 |
| 3 | 黄灯长亮 | 2 s | 过渡警示 |
| 4 | 红灯长亮 | 3 s | 禁止通行 |

**关键文件**：

| 文件 | 说明 |
|------|------|
| `23001010613_lsl_LiSonglun_pedestrian.c` | 主程序，四阶段过街灯逻辑 |
| `iot_gpio_ex.h` | GPIO 引脚功能枚举及接口声明 |
| `hal_iot_gpio_ex.c` | GPIO 扩展功能硬件抽象层实现 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
while (1) {
    // 阶段1：绿灯亮 3s
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE1);
    osDelay(300);

    // 阶段2：绿灯快闪 3 次
    for (int i = 0; i < 3; i++) {
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE0);
        osDelay(30);
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE1);
        osDelay(30);
    }
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE0);

    // 阶段3：黄灯亮 2s
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE1);
    osDelay(200);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE0);

    // 阶段4：红灯亮 3s
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE1);
    osDelay(300);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
}
```

**硬件连接**：红灯接 GPIO_10，绿灯接 GPIO_11，黄灯接 GPIO_12，负极均接 GND。

---

### 6. 设备自检 — `23001010613_lsl_LiSonglun_selfcheck`

**目录**：`23001010613_lsl_LiSonglun_selfcheck/`

**实验目的**：利用 LED 灯直观呈现设备上电自检流程，模拟嵌入式设备 Flash 存储与 Wi-Fi 芯片的检测过程。

**实验内容**：设备上电后按以下顺序执行一次性自检，完成后进入就绪状态：

| 阶段 | 状态 | 时长 | 说明 |
|------|------|------|------|
| 1 | 红灯亮 | 2 s | 模拟检测 Flash 存储 |
| 2 | 黄灯亮 | 2 s | 模拟检测 Wi-Fi 芯片 |
| 3 | 三灯快闪 3 次 | 约 0.3 s | 自检通过提示 |
| 4 | 绿灯长亮 | 持续 | 设备进入就绪状态 |

**关键文件**：

| 文件 | 说明 |
|------|------|
| `23001010613_lsl_LiSonglun_selfcheck.c` | 主程序，上电自检逻辑 |
| `iot_gpio_ex.h` | GPIO 引脚功能枚举及接口声明 |
| `hal_iot_gpio_ex.c` | GPIO 扩展功能硬件抽象层实现 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
// 检测 Flash（红灯）
printf("Checking Flash...\n");
IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE1);
osDelay(200);
IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0);

// 检测 Wi-Fi（黄灯）
printf("Checking Wi-Fi...\n");
IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE1);
osDelay(200);
IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE0);

// 自检通过（三灯快闪）
printf("Self-Check Passed!\n");
for (int i = 0; i < 3; i++) {
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE1);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE1);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE1);
    osDelay(50);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_12, IOT_GPIO_VALUE0);
    osDelay(50);
}

// 就绪（绿灯常亮）
printf("Device Ready!\n");
IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_11, IOT_GPIO_VALUE1);
```

**硬件连接**：红灯接 GPIO_10，绿灯接 GPIO_11，黄灯接 GPIO_12，负极均接 GND。

---

### 7. I2C OLED 英文显示 — `23001010613_lsl_i2coled_en`

**目录**：`23001010613_lsl_i2coled_en/`

**实验目的**：学习 I2C 总线协议与 SSD1306 OLED 显示屏的初始化及驱动使用，在屏幕上显示英文字符串。

**实验内容**：
- 初始化 GPIO_13（SDA）、GPIO_14（SCL）为 I2C0 功能，并启用内部上拉
- 以 400 kHz 波特率初始化 I2C 总线
- 初始化 SSD1306 OLED 芯片，清屏后在坐标 (0, 32) 处显示 `Hello OpenHarmony!`

**关键文件**：

| 文件 | 说明 |
|------|------|
| `23001010613_lsl_i2coled.c` | 主程序，I2C 初始化与 OLED 英文字符串显示 |
| `ssd1306/` | SSD1306 OLED 驱动库 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
hi_i2c_init(HI_I2C_IDX_0, OLED_I2C_BAUDRATE);  // 初始化 I2C，400kHz
ssd1306_Init();
ssd1306_Fill(Black);
ssd1306_SetCursor(0, 32);
ssd1306_DrawString("Hello OpenHarmony!", Font_7x10, White);
ssd1306_UpdateScreen();
```

**硬件连接**：OLED SDA 接 GPIO_13，SCL 接 GPIO_14，VCC 接 3.3V，GND 接地。

---

### 8. I2C OLED 汉字显示 — `23001010613_lsl_i2cOled_cn`

**目录**：`23001010613_lsl_i2cOled_cn/`

**实验目的**：在 SSD1306 OLED 上同时显示英文与中文字符，掌握自定义汉字字库的使用方法。

**实验内容**：
- 初始化 I2C0（GPIO_13/GPIO_14），400 kHz
- 第一行显示英文 `Li Songlun`（Font_7x10）
- 第二行使用 16×16 像素汉字字库依次显示「李」「松」「伦」三个汉字

**关键文件**：

| 文件 | 说明 |
|------|------|
| `23001010613_lsl_i2cOled_cn.c` | 主程序，英汉混排显示逻辑 |
| `ssd1306/` | SSD1306 驱动库（含汉字字库支持） |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
ssd1306_SetCursor(0, 0);
ssd1306_DrawString("Li Songlun", Font_7x10, White);   // 英文

ssd1306_SetCursor(0, 20);
ssd1306_DrawChinese(0, Font_16x16, White);  // 李
ssd1306_DrawChinese(1, Font_16x16, White);  // 松
ssd1306_DrawChinese(2, Font_16x16, White);  // 伦

ssd1306_UpdateScreen();
```

**硬件连接**：OLED SDA 接 GPIO_13，SCL 接 GPIO_14，VCC 接 3.3V，GND 接地。

---

### 9. I2C OLED 图像显示 — `23001010613_lsl_i2cOled_img`

**目录**：`23001010613_lsl_i2cOled_img/`

**实验目的**：掌握在 SSD1306 OLED 上渲染位图图像的方法，实现图文混排显示效果。

**实验内容**：
- 初始化 I2C0（GPIO_13/GPIO_14），400 kHz
- 上半部分：居中显示 64×30 像素的华为 Logo 位图（x=32, y=0）
- 中间：显示英文姓名 `Li Songlun`（x=20, y=36）
- 下方：显示汉字「李松伦」（Font_16x16，x=16, y=48）

**关键文件**：

| 文件 | 说明 |
|------|------|
| `23001010613_lsl_i2cOled_img.c` | 主程序，图像与文字混排显示逻辑 |
| `ssd1306/` | SSD1306 驱动库（含 `DrawImage` 图像渲染接口） |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
ssd1306_DrawImage(gImage_logo, 32, 0, 64, 30);  // 华为 Logo 居中

ssd1306_SetCursor(20, 36);
ssd1306_DrawString("Li Songlun", Font_7x10, White);

ssd1306_SetCursor(16, 48);
ssd1306_DrawChinese(0, Font_16x16, White);  // 李
ssd1306_DrawChinese(1, Font_16x16, White);  // 松
ssd1306_DrawChinese(2, Font_16x16, White);  // 伦

ssd1306_UpdateScreen();
```

**硬件连接**：OLED SDA 接 GPIO_13，SCL 接 GPIO_14，VCC 接 3.3V，GND 接地。

---

### 10. 红绿灯交通场景 — `23001010613_lsl_ledblink_i2coled`

**目录**：`23001010613_lsl_ledblink_i2coled/`

**实验目的**：综合运用 GPIO LED 控制与 I2C OLED 显示，实现红绿灯状态与 OLED 汉字提示的联动，模拟真实交通场景。

**实验内容**：
- 红灯（GPIO_10）与绿灯（GPIO_11）交替亮灭，每次持续 3 s，循环 10 次
- 每次切换时同步刷新 OLED 屏幕：
  - 顶部显示学号 `23001010613`
  - 左侧显示「红灯亮/灭」「绿灯亮/灭」汉字状态（Font_16x16）
  - 右下角显示爱心图标（32×32 位图）
- 循环结束后双灯熄灭，OLED 清屏

**关键文件**：

| 文件 | 说明 |
|------|------|
| `23001010613_lsl_ledblink_i2coled.c` | 主程序，LED 与 OLED 联动控制逻辑 |
| `ssd1306/` | SSD1306 驱动库（含交通场景汉字字库及爱心位图） |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
for (int i = 0; i < 10; i++) {
    // 红灯亮，绿灯灭
    IoTGpioSetOutputVal(GPIO_RED,   IOT_GPIO_VALUE1);
    IoTGpioSetOutputVal(GPIO_GREEN, IOT_GPIO_VALUE0);
    oled_show_state(0);   // OLED: 红灯亮 / 绿灯灭
    osDelay(300);         // 3 秒

    // 绿灯亮，红灯灭
    IoTGpioSetOutputVal(GPIO_RED,   IOT_GPIO_VALUE0);
    IoTGpioSetOutputVal(GPIO_GREEN, IOT_GPIO_VALUE1);
    oled_show_state(1);   // OLED: 红灯灭 / 绿灯亮
    osDelay(300);         // 3 秒
}
// 全灭并清屏
oled_show_state(2);
ssd1306_Fill(Black);
ssd1306_UpdateScreen();
```

**OLED 显示布局**：

| 区域 | 内容 |
|------|------|
| 顶部 (y=0) | 学号 `23001010613`（Font_7x10） |
| 左侧 (y=24) | 「红灯亮」或「红灯灭」（Font_16x16） |
| 左侧 (y=42) | 「绿灯亮」或「绿灯灭」（Font_16x16） |
| 右下角 (x=96, y=32) | 爱心图标（32×32 位图） |

**硬件连接**：红灯接 GPIO_10，绿灯接 GPIO_11，负极接 GND；OLED SDA 接 GPIO_13，SCL 接 GPIO_14，VCC 接 3.3V。

---

### 11. 按键检测 — `23001010613_lsl_demo_key_gpio`

**目录**：`23001010613_lsl_demo_key_gpio/`

**实验目的**：掌握 GPIO 输入模式与按键消抖的实现方法，通过检测短按/长按事件控制 LED 灯状态。

**实验内容**：
- GPIO_5 配置为输入模式并启用内部上拉，作为按键输入
- GPIO_9 配置为输出模式，控制 LED
- **短按**（按下时长 < 3 s）：切换 LED 亮/灭状态
- **长按**（按下时长 ≥ 3 s）：LED 快速闪烁 3 次后恢复原状态

**关键文件**：

| 文件 | 说明 |
|------|------|
| `entry.c` | 主程序，任务入口，LED 控制与按键事件响应 |
| `key.c` | 按键驱动，含消抖与短按/长按识别逻辑 |
| `key.h` | 按键驱动头文件，定义按键 GPIO 及事件常量 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
// key.c — 短按/长按识别
if ((val == 0) && (is_pressed == RELEASED)) {
    osDelay(1);  // 消抖
    IoTGpioGetInputVal(KEY_GPIO, &val);
    if (val == 0) {
        is_pressed = PRESSED;
        keyPressTime = osKernelGetTickCount();
    }
} else if ((val == 1) && (is_pressed == PRESSED)) {
    is_pressed = RELEASED;
    if ((osKernelGetTickCount() - keyPressTime) >= 300) {
        return KEY_PRESSLONG_kernel;  // 长按
    }
    return KEY_PRESS_kernel;          // 短按
}

// entry.c — 事件响应
if (key_val == KEY_PRESS_kernel) {
    led_state = !led_state;           // 短按切换
    IoTGpioSetOutputVal(LED_GPIO, led_state ? IOT_GPIO_VALUE1 : IOT_GPIO_VALUE0);
} else if (key_val == KEY_PRESSLONG_kernel) {
    for (int i = 0; i < 3; i++) {    // 长按闪烁 3 次
        IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE1); osDelay(10);
        IoTGpioSetOutputVal(LED_GPIO, IOT_GPIO_VALUE0); osDelay(10);
    }
}
```

**硬件连接**：按键一端接 GPIO_5，另一端接 GND；LED 正极接 GPIO_9，负极接 GND。

---

### 12. ADC 按键检测 — `23001010613_lsl_key_adc`

**目录**：`23001010613_lsl_key_adc/`

**实验目的**：掌握 ADC（模数转换）按键的读取方式，通过采集 ADC2 通道电压值区分不同按键，并控制 LED 灯状态。

**实验内容**：
- 通过 `hi_adc_read()` 读取 ADC_CHANNEL_2 的采样值，按电压范围区分三路按键：
  - **Key0**（ADC 5～227）：核心板自带按键，短按/长按打印日志
  - **S1**（ADC 228～454）：短按点亮 LED，长按关闭 LED
  - **S2**（ADC 455～681）：短按进入 LED 闪烁模式，长按关闭 LED
- LED（GPIO_9，低电平有效）支持三种状态：常亮、常灭、500 ms 周期闪烁

**ADC 按键电压范围**：

| 按键 | ADC 采样值范围 | 短按返回值 | 长按返回值 |
|------|--------------|-----------|-----------|
| Key0 | 5 ～ 227 | `KEY_PRESS_kernel`(1) | `KEY_PRESSLONG_kernel`(2) |
| S1   | 228 ～ 454 | `KEY1_PRESS`(3) | `KEY1_PRESSLONG`(4) |
| S2   | 455 ～ 681 | `KEY2_PRESS`(5) | `KEY2_PRESSLONG`(6) |
| 无按键 | > 682 | — | — |

**关键文件**：

| 文件 | 说明 |
|------|------|
| `entry.c` | 主程序，LED 状态机与按键事件响应 |
| `adckey.c` | ADC 按键驱动，三路按键读取与短/长按识别 |
| `adckey.h` | ADC 按键头文件，定义通道号及事件常量 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
// S1 短按点亮，长按关闭
key_val = readAdCKey1();
if (key_val == KEY1_PRESS)     ledState = LED_STATE_ON;
else if (key_val == KEY1_PRESSLONG) ledState = LED_STATE_OFF;

// S2 短按闪烁，长按关闭
key_val = readAdCKey2();
if (key_val == KEY2_PRESS)     ledState = LED_STATE_BLINK;
else if (key_val == KEY2_PRESSLONG) ledState = LED_STATE_OFF;
```

**硬件连接**：ADC 按键模块信号端接 GPIO_5（ADC_CHANNEL_2）；LED 正极接 GPIO_9，负极接 GND。

---

### 13. ADC 按键切换 OLED 显示（进阶）— `23001010613_lsl_adckey_oled`

**目录**：`23001010613_lsl_adckey_oled/`

**实验目的**：综合运用 ADC 按键、I2C OLED、多线程，实现按键翻页切换六个不同显示页面，并实时采集展示 ADC 电压值，同时支持 Key0 在特定页面进行计数和 LED 控制。

**实验内容**：
- 双线程架构：**按键线程**处理翻页与 Key0 交互，**ADC 采集线程**（100 ms 刷新）实时更新电压数据
- S1 短按向后翻页，S2 短按向前翻页，共 **6 个页面**循环切换：

| 页面 | 内容 | Key0 交互 |
|------|------|-----------|
| 1 | 学号 `23001010613`、英文名 `Li Songlun`、汉字「李松伦」 | — |
| 2 | 爱心图标（居中）+ 学号 | — |
| 3 | `Hello HarmonyOS!` / `Li Songlun` / `NEUSOFT 2024` | — |
| 4 | ADC 原始值 + 实时电压（V = ADC × 1.8 / 4096） | — |
| 5 | Key0 按键计数，每次短按计数 +1 | 计数 +1 并刷新显示 |
| 6 | LED 状态显示（ON / OFF） | 切换 GPIO_9 LED |

**关键文件**：

| 文件 | 说明 |
|------|------|
| `entry.c` | 主程序，OLED 初始化、六页显示函数、双线程入口 |
| `adckey.c` | ADC 按键驱动（同实验12） |
| `adckey.h` | ADC 按键头文件 |
| `ssd1306/` | SSD1306 OLED 驱动库 |
| `BUILD.gn` | GN 构建配置文件 |

**核心代码片段**：
```c
// S1 向后翻页（1→6 循环），S2 向前翻页（6→1 循环）
if (readAdCKey1() == KEY1_PRESS) {
    if (++g_flag_show == 7) g_flag_show = 1;
    oled_control(g_flag_show);
}
if (readAdCKey2() == KEY2_PRESS) {
    if (--g_flag_show == 0) g_flag_show = 6;
    oled_control(g_flag_show);
}

// Key0：第5页计数，第6页控制 LED
if (readAdCKey0() == KEY_PRESS_kernel) {
    if (g_flag_show == 5) { g_key0_count++; oled_showKeyCount(); }
    else if (g_flag_show == 6) {
        g_led_state = !g_led_state;
        IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_9,
            g_led_state ? IOT_GPIO_VALUE0 : IOT_GPIO_VALUE1);
        oled_showLedState();
    }
}
```

**硬件连接**：ADC 按键模块信号端接 GPIO_5；OLED SDA 接 GPIO_13，SCL 接 GPIO_14，VCC 接 3.3V，GND 接地。

---

## 工程构建说明

根目录 `BUILD.gn` 用于选择当前激活的实验项目，取消对应行的注释即可切换：

```gn
lite_component("app") {
    features = [
        # "lsl613_helloworld:lsl613_helloworld",
        # "lsl613_ledcontrol:lsl613_ledcontrol",
        # "23001010613_lsl_LiSonglun_blueled_blink:23001010613_lsl_LiSonglun_blueled_blink",
        # "23001010613_lsl_LiSonglun_ledblink:23001010613_lsl_LiSonglun_ledblink",
        # "23001010613_lsl_LiSonglun_pedestrian:23001010613_lsl_LiSonglun_pedestrian",
        # "23001010613_lsl_LiSonglun_selfcheck:23001010613_lsl_LiSonglun_selfcheck",
        # "23001010613_lsl_i2coled_en:i2coled",
        # "23001010613_lsl_i2cOled_cn:i2cOled_cn",
        # "23001010613_lsl_i2cOled_img:i2cOled_img",
        # "23001010613_lsl_ledblink_i2coled:ledblink_i2coled",
        # "23001010613_lsl_demo_key_gpio:demo_key_gpio",
        # "23001010613_lsl_key_adc:demo_key_adc",
        "23001010613_lsl_adckey_oled:adckey_oled",
    ]
}
```

完整编译烧录流程请参考 OpenHarmony 官方文档：[Hi3861 快速上手](https://gitee.com/openharmony/docs/blob/master/zh-cn/device-dev/quick-start/quickstart-lite-steps-hi3861-building.md)

---

## 作者

- **姓名**：李松伦
- **学号**：23001010613
- **课程**：鸿蒙系统开发

---

## 许可证

本项目基于 [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0) 开源协议。
