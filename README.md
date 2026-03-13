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
        "23001010613_lsl_LiSonglun_selfcheck:23001010613_lsl_LiSonglun_selfcheck",
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
