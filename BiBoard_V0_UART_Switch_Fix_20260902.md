# BiBoard V0.x 离线语音与第二串口切换问题修复记录

**日期：** 2026-09-02  
**适用板型：** BiBoard V0.1、BiBoard V0.2  
**相关提交：** `f22ebddbf9435e00a7207a84a87c2e6f3720ccdd`  
**涉及文件：** `src/moduleManager.h`、`src/voice.h`、`src/reaction.h`

## 1. 问题背景

BiBoard V0.1/V0.2 的离线语音模块和 Grove 第二串口共用 ESP32 的 `Serial2`：

| 模式 | 模块代码 | UART | 引脚 | 波特率 |
|---|---|---|---|---|
| Grove 第二串口 / Xiaozhi | `S` | `Serial2` | RX16 / TX17 | 115200 |
| 离线语音 | `A` | `Serial2` | RX16 / TX17 | 9600 |

由于两种模式共用 UART 和物理引脚，它们在 V0.1/V0.2 上必须互斥，不能同时激活。

主要现象包括：

- 串口监视器反复打印 `Undefined token!` 和乱码字符；
- `X?` 查询曾显示 `S=1、A=1`；
- 从离线语音切换到第二串口后，必须复位主板才能恢复；
- Xiaozhi 发送 `kwkF 3` 时，主板有时把剩余的 `w...` 当成 Wi-Fi 命令；
- 模式状态显示正确，但 UART 实际波特率或驱动状态不正确；
- 切换过程中可能出现错误动作或无效命令反馈循环。

## 2. 根因分析

### 2.1 同一 UART 被两个模式用不同波特率初始化

Grove Serial 使用 115200，Voice 使用 9600。当 `S` 和 `A` 同时激活时，后初始化的模块会覆盖前一个模块的 UART 配置。Xiaozhi 的 115200 数据随后会被 9600 接收器解释成随机字节，并进入命令解析器。

模块状态会保存在 Flash/NVS 中。启动时读取的 `moduleState` 会覆盖源码默认值，因此普通固件上传不会自动清除以前保存的 `S=1、A=1` 状态。

### 2.2 启动时 `voiceStop()` 会误关 Grove Serial

当保存状态为 `S=1、A=0` 时，原初始化流程会：

1. 以 115200 启动 `Serial2`；
2. 因 Voice 未激活而调用 `voiceStop()`；
3. `voiceStop()` 把同一个 `Serial2` 改为 9600，发送 `XAd`，然后执行 `end()`。

结果是 Grove Serial 的状态标志虽然为 1，但 UART 已被 Voice 的关闭流程终止。

### 2.3 热切换时向错误设备发送 `XAd`

如果先把滑动开关拨到 Xiaozhi，再发送 `XS`，原代码在关闭 Voice 时仍会以 9600 向当前已经接通的 Xiaozhi 发送 `XAd`。Xiaozhi 工作在 115200，可能将这些错误波特率数据解释为乱码并产生异常响应。

### 2.4 10 ms 组帧超时可能截断 Xiaozhi 命令

原串口命令组帧超时为 10 ms。如果 Xiaozhi 分两次写入首字节 `k` 和后续 `wkF 3`，且两次写入间隔超过 10 ms，主板会先处理孤立的 `k`，再把剩余的 `w` 当成 Wi-Fi token，因此打印 Wi-Fi 参数提示而不是执行步态命令。

### 2.5 错误信息回发会放大乱码

原来的默认命令分支使用 `printToAllPorts()` 输出 `Undefined token!`。该文本也会被发送回 Grove Serial；当外部 UART 桥接设备存在回显时，一个无效字节可能演变为持续的错误反馈循环。

### 2.6 机械开关切换期间存在瞬态数据

机械开关切换、GPIO 重新挂接以及 9600/115200 波特率转换期间，UART RX 可能采样到 CR、LF、NUL、高位乱码或其他不完整字节。如果不清理 RX 缓冲区，这些字节会被当作新命令的 token。

## 3. 代码修改

### 3.1 `src/moduleManager.h`

#### Grove Serial 完整重启

启动第二串口前执行完整切换：

```cpp
Serial2.end();
delay(20);
Serial2.begin(115200, SERIAL_8N1, UART_RX2, UART_TX2);
```

V0.1/V0.2 在重新初始化后增加 100 ms 隔离窗口，并持续清空 RX，避免物理开关和波特率切换期间的残留数据进入解析器。

#### V0.x 模式互斥

互斥逻辑只应用于 `BiBoard_V0_1` 和 `BiBoard_V0_2`：

- `XA`：自动关闭 Grove Serial，再启用 Voice；
- `XS`：自动关闭 Voice，再启用 Grove Serial。

BiBoard V1.0、BiBoard2 和其他板型保持原有切换逻辑。

#### 静默关闭 Voice

V0.1/V0.2 从 Voice 切换到 Grove Serial 时调用：

```cpp
voiceStop(false);
```

该调用只关闭共享 UART，不再向已经可能连接到 Xiaozhi 的线路发送 9600 波特率的 `XAd`。

#### 修复启动初始化顺序

当 V0.1/V0.2 保存状态为 `S=1、A=0` 时，不再调用 `voiceStop()`，避免关闭刚刚启动的 Grove Serial。

#### 提高切换命令可靠性

- V0.1/V0.2 切换期间让 USB 串口控制命令优先于共享的 `Serial2`；
- 本轮已经收到 USB/串口命令时，不再继续使用旧 Voice 配置读取同一个 UART；
- Grove Serial 帧头丢弃 CR、LF、NUL 和非可打印 ASCII 字节；
- Grove Serial 普通命令组帧超时由 10 ms 提高到 100 ms，避免 `kwkF 3` 被拆成 `k` 和 `wkF 3`。

### 3.2 `src/voice.h`

V0.x 的 Voice 初始化改为显式指定共享 UART 引脚：

```cpp
SERIAL_VOICE.begin(9600, SERIAL_8N1, UART_RX2, UART_TX2);
```

初始化后清空切换期间残留的 RX 字节。

`voiceStop()` 增加可选参数：

```cpp
void voiceStop(bool notifyVoiceModule = true);
```

- `voiceStop(true)`：保持原行为，发送 `XAd` 后关闭 UART；
- `voiceStop(false)`：静默关闭 UART，不发送 `XAd`。

### 3.3 `src/reaction.h`

无效 token 的诊断信息改为只输出到 USB 调试串口：

```cpp
PTLF("Undefined token!");
PTL(newCmd);
```

不再通过 `printToAllPorts()` 把错误信息发回 Xiaozhi，从而阻断串口回显反馈循环。

## 4. 推荐切换方式

### 4.1 最稳妥流程

无论切换方向，推荐使用：

```text
发送 X → 确认 S=0/A=0 → 拨动开关 → 等待约 200 ms → 发送 XA 或 XS
```

离线语音切换到第二串口：

```text
X → 拨到第二串口档 → 等待约 200 ms → XS
```

第二串口切换到离线语音：

```text
X → 拨到离线语音档 → 等待约 200 ms → XA
```

该流程在 UART 已关闭的情况下拨动机械开关，最不容易产生错误电平或错波特率数据。

### 4.2 便捷流程

修改后的代码也支持直接切换：

```text
先拨到目标档位 → 随即发送 XA 或 XS
```

- 拨到离线语音档后立即发送 `XA`；
- 拨到第二串口档后立即发送 `XS`。

不建议先发送目标命令、过一段时间后再拨开关，因为 UART 会立即改变波特率，而物理线路仍可能连接着旧模块。

## 5. 编译错误及修复

在添加 Grove Serial 帧头过滤逻辑时，`read_serial()` 内的 `if (fromGroveSerial)` 分支曾遗漏一个右花括号 `}`，导致后续函数全部被解析为嵌套函数，并产生大量连锁错误：

```text
function-definition is not allowed here
expected unqualified-id
mbedtls_* does not name a type
expected '}' at end of input
```

缺失括号已经补齐。mbedTLS、WebSocket 和 WiFiClientSecure 相关报错均为语法错误引发的连锁反应，并非库本身损坏。

## 6. 编译配置与验证结果

验证环境：

```text
Board FQBN: esp32:esp32:esp32
ESP32 Arduino Core: 2.0.12
Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
```

最终完整编译结果：

```text
Sketch uses 1751557 bytes (55%) of program storage space.
Maximum is 3145728 bytes.

Global variables use 71112 bytes (21%) of dynamic memory.
Leaving 256568 bytes for local variables.
Maximum is 327680 bytes.
```

使用默认分区会失败，因为默认应用区只有 1310720 bytes：

```text
Sketch uses 1751557 bytes (133%) of program storage space.
Sketch too big.
Error during build: text section exceeds available space in board
```

因此 Arduino IDE 必须选择：

```text
工具 → Partition Scheme → Huge APP (3MB No OTA/1MB SPIFFS)
```

## 7. 预期运行日志

从 Voice 直接切换到 Grove Serial 时，预期看到：

```text
- disable       Voice
Turn off the audio response
+ enable        Grove_Serial
Start Serial2
S, A, T, L, D, I, B, U, G, C, Q,
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
```

不应再看到 Voice 关闭阶段向 Xiaozhi 发送：

```text
Voice Serial2 @9600
Voice TX: X Ad
```

收到 `kwkF 3` 后应正常打印并执行：

```text
wkF
Cycle counting mode: 3 cycles
```

## 8. 后续排查建议

如果更新固件后，设备在不切换模式、尤其是舵机动作期间仍持续收到乱码，应继续检查硬件层：

- Xiaozhi 与 BiBoard 是否可靠共地；
- UART 是否为 115200、8N1；
- UART 电平是否为 3.3 V TTL；
- Grove 连接线是否过长或靠近舵机动力线；
- 舵机动作时电源是否明显压降；
- 使用 USB-TTL 或逻辑分析仪直接捕获 Xiaozhi TX，确认其实际发送内容。

这类硬件噪声无法仅靠软件过滤完全消除，但本次修改可以避免切换瞬态、错波特率数据和错误回显进一步放大问题。
