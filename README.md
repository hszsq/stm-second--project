# 电控组第二次培训作业
* **韩松志**
* ***20263003390***

> 题目：使用 CLion + CubeMX 完成三个 LED 的流水灯控制，并将工程代码与说明文档提交至 GitHub

## 一、任务完成情况

| 要求 | 完成情况 |
|---|---|
| CubeMX 建工程，配置三个 LED 的 GPIO | ✅ PA0 / PA1 / PA2 |
| User Label 命名为 LED0 / LED1 / LED2 | ✅ 已配置，宏由 CubeMX 生成 |
| CLion 打开工程并编写代码 | ✅ CMake 工程，CLion 直接打开编译 |
| 间隔 500 ms 三灯流水灯（基础任务） | ✅ 已实现并烧录验证 |
| 加分项：摆脱 `HAL_Delay()`，非阻塞延时 | ✅ `HAL_GetTick()` 软定时 + 状态机 |
| GitHub 公开仓库 + README + 视频 | ✅ 见仓库 |

## 二、硬件与引脚

| LED | 引脚 | CubeMX User Label | 接线 |
|---|---|---|---|
| LED0 | PA0 | `LED0` | PA0 → 220Ω → LED 阳极，阴极 → GND |
| LED1 | PA1 | `LED1` | 同上 |
| LED2 | PA2 | `LED2` | 同上 |

- MCU：**STM32F103C8T6**（LQFP48，实测 Flash 128 KiB）
- 时钟：HSE 8 MHz → PLL ×9 → **SYSCLK 72 MHz**（APB1 36 MHz，APB2 72 MHz）
- 调试口：SWD（PA13 / PA14）
- 工具链：**CMake + arm-none-eabi-gcc**，IDE 为 CLion

CubeMX 按 User Label 自动在 `Core/Inc/main.h` 生成：

```c
#define LED0_Pin GPIO_PIN_0
#define LED0_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_2
#define LED2_GPIO_Port GPIOA
```

代码全程直接调用这些宏，不出现硬编码的 `GPIOA` / `GPIO_PIN_x`。

CubeMX 中三个引脚配置：`GPIO_Output`、推挽输出、无上下拉、低速、初始电平 Low。

## 三、实现思路

### 3.1 基础版：阻塞延时（`HAL_Delay`）

主循环按 `LED0 → LED1 → LED2` 顺序点亮，每次只亮一颗、其余熄灭，
切换后用 `HAL_Delay(500)` 等待 500 ms，形成流水效果。

```c
static uint8_t step = 0U;
LED_OFF(LED0_GPIO_Port, LED0_Pin);
LED_OFF(LED1_GPIO_Port, LED1_Pin);
LED_OFF(LED2_GPIO_Port, LED2_Pin);
switch (step) {
  case 0U:  LED_ON(LED0_GPIO_Port, LED0_Pin); break;
  case 1U:  LED_ON(LED1_GPIO_Port, LED1_Pin); break;
  default:  LED_ON(LED2_GPIO_Port, LED2_Pin); break;
}
step = (uint8_t)((step + 1U) % 3U);
HAL_Delay(500);
```

**缺点**：`HAL_Delay()` 是忙等，等待期间 CPU 被独占，无法处理其它任务。
参考实现留档在 `appendix/basic_version_snippet.c`。

### 3.2 加分项：非阻塞延时（`HAL_GetTick()` + 状态机）

不使用 `HAL_Delay()`。`HAL_GetTick()` 返回 SysTick 中断每 1 ms 递增一次的
毫秒时间戳。主循环每轮只做一次"时间差比较"：

- 若 `HAL_GetTick() - s_last_tick >= 500` → 切换下一个灯，并更新 `s_last_tick`
- 否则**立即返回**，主循环继续高速空转，不阻塞任何逻辑

这样延时与业务解耦，500 ms 的等待时间里 CPU 可以完全自由地处理其它任务
（本工程为演示只做空转，工程上加按键扫描、串口协议解析都不受影响）。

**两个实现细节：**

1. **时间戳用累加而不是直接赋值**
   ```c
   s_last_tick += FLOW_INTERVAL_MS;   // 正确
   // s_last_tick = HAL_GetTick();    // 错误：把循环耗时算进去，误差逐拍累积
   ```
   每轮循环本身有微秒级耗时，若直接赋值为当前时间，周期会逐渐变长；
   累加则长期没有累积误差。
2. **首拍初始化**
   `s_last_tick` 初值为 0，若直接比较会立刻触发一次切换（第一颗灯一闪而过）。
   故进入循环先判断 `s_last_tick == 0`，记录一次当前时间并点亮第一颗灯。

## 四、核心代码（非阻塞版，当前工程生效）

```c
/* USER CODE BEGIN 0 */
#define FLOW_INTERVAL_MS    500U

#define LED_ON(port, pin)   HAL_GPIO_WritePin((port), (pin), GPIO_PIN_SET)
#define LED_OFF(port, pin)  HAL_GPIO_WritePin((port), (pin), GPIO_PIN_RESET)

static uint32_t s_last_tick = 0U;   /* 上次切换的时间戳(ms) */
static uint8_t  s_step      = 0U;   /* 当前点亮第几个灯 0~2 */

/* 只点亮 idx 号灯，其余熄灭 */
static void LED_ShowOnly(uint8_t idx)
{
  LED_OFF(LED0_GPIO_Port, LED0_Pin);
  LED_OFF(LED1_GPIO_Port, LED1_Pin);
  LED_OFF(LED2_GPIO_Port, LED2_Pin);

  switch (idx)
  {
    case 0U:  LED_ON(LED0_GPIO_Port, LED0_Pin); break;
    case 1U:  LED_ON(LED1_GPIO_Port, LED1_Pin); break;
    default:  LED_ON(LED2_GPIO_Port, LED2_Pin); break;
  }
}
/* USER CODE END 0 */
```

```c
/* USER CODE BEGIN 3  —— while(1) 内 */
/* 首拍初始化 */
if (s_last_tick == 0U)
{
  s_last_tick = HAL_GetTick();
  LED_ShowOnly(s_step);
}

/* 非阻塞：每 500ms 才切换一次，其余时间主循环照常运行 */
if ((HAL_GetTick() - s_last_tick) >= FLOW_INTERVAL_MS)
{
  s_last_tick += FLOW_INTERVAL_MS;          /* 累加，消除累计误差 */
  s_step = (uint8_t)((s_step + 1U) % 3U);
  LED_ShowOnly(s_step);
}
/* USER CODE END 3 */
```

> 极性：以上按「高电平点亮」编写。若 LED 为低电平点亮（如蓝 Pill 板载灯），
> 将 `LED_ON` / `LED_OFF` 宏里的 `GPIO_PIN_SET` 与 `GPIO_PIN_RESET` 对调即可。

## 五、编译与烧录

### CLion
打开工程根目录即可（CLion 自动读取 `CMakePresets.json` 的 **Debug** 预设）：

- 编译：`构建` → `重新构建项目`（`Ctrl+Shift+F9`），或 `构建项目`（`Ctrl+F9`）
- 成功标志：`[2/2] Linking C executable led-0.elf` + `构建 已完成`

### 命令行（等价）

```bash
cmake --preset Debug
cmake --build --preset Debug
```

### 烧录（ST-Link + OpenOCD）

工程根目录提供 `flash.bat`，双击即可；等价命令：

```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
  -c "program build/Debug/led-0.elf verify reset exit"
```

成功标志：`** Programming Finished **` → `** Verified OK **` → `** Resetting Target **`

产物（`build/Debug/`）：

| 文件 | 说明 |
|---|---|
| `led-0.elf` | 带调试信息，CLion 烧录 / 调试 |
| `led-0.hex` | STM32CubeProgrammer / ST-Link Utility |
| `led-0.bin` | 裸二进制 |

资源占用（非阻塞版）：**FLASH 4736 B / 64 KB（7.23%）**，
**RAM 1592 B / 20 KB（7.77%）**。

## 六、验收视频

`video/` 目录下为两个代码版本的验收录像（CLion 编译 → 烧录 → 板级流水灯）：

| 文件 | 内容 |
|---|---|
| `video/b626f830886479f3851f9c0862f31208.mp4` | 验收录像 1 |
| `video/820cae7a1476dd29dc34d14d129d06c6.mp4` | 验收录像 2 |

## 七、目录结构

```
led-0/
├── Core/                     # CubeMX 生成的源码（main.c / gpio.c / main.h ...）
├── Drivers/                  # CMSIS + STM32F1xx HAL 驱动
├── cmake/                    # 工具链文件（已改为绝对路径，不依赖系统 PATH）
├── appendix/
│   ├── basic_version_snippet.c   # 基础版 HAL_Delay 实现留档
│   └── main_nonblocking.c        # 非阻塞版完整 main.c 备份
├── video/                    # 验收录像
├── CMakeLists.txt            # CMake 工程
├── CMakePresets.json         # Debug / Release 预设
├── led-0.ioc                 # CubeMX 工程配置（三个 LED 的 User Label 在此）
├── flash.bat                 # 一键烧录脚本（OpenOCD + ST-Link）
├── startup_stm32f103xb.s     # 启动文件
└── STM32F103XX_FLASH.ld      # 链接脚本
```

## 八、遇到的问题与解决

1. **CLion 报 `CMAKE_C_COMPILER not found`**
   原因：CubeMX 生成的工具链文件用 `arm-none-eabi-gcc`（仅名字），依赖系统 PATH，
   而本机未把 STM32 工具链加入 PATH。
   解决：把 `cmake/gcc-arm-none-eabi.cmake` 的编译器路径改为**绝对路径**，
   并在**剔除 PATH** 的环境下重新配置验证通过。
2. **`ninja: no work to do`，看起来像没编译**
   原因：源文件时间戳早于目标文件（例如用 `Copy-Item` 覆盖源文件会保留旧时间戳）。
   解决：修改源文件时间戳，或使用 `重新构建项目`（全量重编）。
3. **直接用 `▶ 运行` 会报 `could not connect (error 138)`**
   原因：Run 配置会连本地 GDB Server（3333），而烧录并不需要调试器。
   解决：烧录统一走 `flash.bat`（OpenOCD 直接下载并校验），不使用 Run 配置。
