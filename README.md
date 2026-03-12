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

## 工程构建说明

根目录 `BUILD.gn` 用于选择当前激活的实验项目，修改 `features` 字段即可切换：

```gn
lite_component("app") {
    features = [
        # 切换到 Hello World 实验时取消注释下一行，并注释 ledcontrol 行
        # "lsl613_helloworld:lsl613_helloworld",
        "lsl613_ledcontrol:lsl613_ledcontrol",
    ]
}
```

完整编译烧录流程请参考 OpenHarmony 官方文档：[Hi3861 快速上手](https://gitee.com/openharmony/docs/blob/master/zh-cn/device-dev/quick-start/quickstart-lite-steps-hi3861-building.md)

---

## 作者

- **姓名**：李松伦
- **学号**：lsl23001010613
- **课程**：鸿蒙系统开发

---

## 许可证

本项目基于 [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0) 开源协议。
