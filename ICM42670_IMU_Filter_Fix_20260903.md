# BiBoard V1 ICM42670 姿态异常问题分析与修复记录

## 1. 文档信息

- 日期：2026-09-03
- 项目：OpenCatEsp32
- 机器人型号：Bittle
- 主板：BiBoard V1.0
- IMU 芯片：ICM42670
- 主程序：`OpenCatEsp32.ino`
- 主板宏：`#define BiBoard_V1_0`

## 2. 问题现象

为 BiBoard V1 主板编译并上传固件后，机器人会持续判断身体姿势异常，并反复执行 `lifted`、`dropped`、`dropRec`、`rc`、`wkF`、`bkF` 等恢复或调整动作。

即使发送 `gc` 命令重新校准 IMU，问题仍然存在，并且校准结束、IMU 任务恢复后可能立即报告翻倒。

典型日志如下：

```text
ICM: -0.40 -0.42 10.27   -4.5   10.7    0.1
ICM: -0.15  0.08  9.67  184.0   20.5  174.0
ICM: -0.42 -0.49  9.84  196.8  -61.2 -153.1
```

其中前三列为三轴加速度，后三列为 yaw、pitch、roll。机器人静止时，加速度模长接近 `9.8~10 m/s²`，说明传感器读数基本正常；但姿态角会在接近 `0°` 和 `±180°` 之间跳变，表明问题主要发生在姿态融合算法，而不是简单的传感器零偏。

执行 `gc` 后的关键日志：

```text
19:02:40 -> Setting updateGyroQ to false...
19:02:48 -> New ICM offsets: 561.18 77.32 362.86 -44.99 -0.25 -19.01
19:02:52 -> Creating IMU task...
19:02:53 -> ICM: -1.84 6.36 -4.66 197.0 -86.7 171.4
19:02:53 -> EXCEPTION: Fall over
```

校准后的 offset 与校准前数据接近，说明 ICM42670 硬件和静态零偏不是此次故障的主要原因。

## 3. 根因分析

### 3.1 首次采样使用了过大的 `deltaT`

ICM42670 的 Madgwick 滤波器按下面的方式计算积分时间：

```cpp
now = micros();
deltaT = (now - lastUpdate) / 1000000.0f;
lastUpdate = now;
```

原代码没有在 IMU 初始化时正确建立 `lastUpdate` 时间基准。因此，冷启动后的第一次更新可能把 ESP32 启动至首次 IMU 读取之间的数秒时间全部作为一个积分周期。

正常采样周期约为数毫秒，而异常首帧可能达到数秒。Madgwick 算法中的四元数校正量直接乘以 `deltaT`，因此首帧会产生远大于正常值的积分，将四元数推到错误姿态。

### 3.2 `gc` 后没有重置滤波器状态

执行 `gc` 时，IMU 任务会停止十余秒用于提示、采样和写入校准值。原实现只更新传感器 offset，没有重置以下姿态融合状态：

- 四元数 `q[4]`；
- `lastUpdate` 和 `deltaT`；
- yaw 漂移补偿；
- pitch/roll 均值滤波历史；
- Madgwick 陀螺仪 bias；
- 全局姿态值及上一帧异常状态。

任务恢复后的第一帧会把整个校准暂停时间作为 `deltaT` 再次积分，因此 `gc` 无法修复姿态，反而可能立即产生 `±180°` 级跳变。

### 3.3 Madgwick 梯度归一化缺少零值保护

当传感器接近水平且当前四元数接近正确解时，校正梯度可能为零或非常小。原代码直接执行：

```cpp
hatDot1 /= norm;
hatDot2 /= norm;
hatDot3 /= norm;
hatDot4 /= norm;
```

这可能产生除零、`NaN` 或数值放大。一旦非有限值进入四元数，后续姿态输出和异常检测都会失效。

### 3.4 `asin()` 输入缺少浮点边界保护

由四元数计算 pitch 时，理论输入范围是 `[-1, 1]`。浮点误差可能令输入略微越界，使 `asin()` 返回 `NaN`。

### 3.5 `gc` 流程重复计算 offset

原校准任务先调用一次 `icm.getOffset(200)`，随后 `calibrateICM()` 清零 offset 并再次采样 200 次。

第一次调用前没有清空原 offset，因此日志中所谓 `Current IMU Offsets` 实际上是旧 offset 与新采样累计后再除以 200 的结果，不是真正保存的当前值。第二次调用才是有效校准。

### 3.6 校准后的“稳定等待”期间 IMU 尚未运行

原流程先等待 3 秒，随后才调用 `createIMUTask()`。因此日志虽然提示正在等待 IMU 稳定，但这 3 秒内并没有进行姿态采样和滤波。

### 3.7 错误姿态触发动作正反馈

错误的 pitch、roll 和瞬态加速度会触发 lifted、flipped、pushed 或 knocked 等异常。机器人执行恢复动作后又产生真实加速度，进一步触发新的异常，最终表现为持续调整。

## 4. 代码修改

### 4.1 增加完整滤波器重置接口

修改文件：

- `src/icm42670/petoi_icm42670p.h`
- `src/icm42670/petoi_icm42670p.cpp`

新增 `resetFusion()`，统一重置：

- yaw、pitch、roll；
- yaw drift；
- pitch/roll 均值滤波历史；
- 四元数为单位四元数 `{1, 0, 0, 0}`；
- Madgwick gyro bias；
- `lastUpdate`、`firstUpdate`、`deltaT`；
- 首帧时间基准状态。

`init()` 完成传感器量程和频率配置后调用该接口，`calibrateICM()` 完成 offset 校准后也调用该接口。

### 4.2 首帧只建立时间基准

新增 `timingReady` 状态。滤波器重置后的第一组有效数据只执行：

```cpp
lastUpdate = firstUpdate = now;
timingReady = true;
return;
```

这一帧不参与四元数积分，从而避免把开机时间或校准暂停时间作为积分周期。

### 4.3 过滤异常积分周期

增加 `deltaT` 有效性判断：

```cpp
if (!isfinite(deltaT) || deltaT <= 0.0f || deltaT > 0.1f) {
  return;
}
```

超过 100 ms 的陈旧数据、任务暂停或 I²C 长时间阻塞不会再被送入姿态积分。由于 `lastUpdate` 已更新，下一帧可以从新的时间基准正常恢复。

### 4.4 增加数值稳定性保护

Madgwick 梯度归一化前检查模长是否为有限值且大于极小阈值。梯度为零时，将校正梯度设为零，保留陀螺仪积分，避免除零污染四元数。

四元数归一化前同样检查模长。如果四元数已经无效，则调用 `resetFusion()` 恢复到安全初始状态。

计算 pitch 前把 `asin()` 输入限制到 `[-1, 1]`：

```cpp
float sinPitch = 2.0f * (q[1] * q[3] - q[0] * q[2]);
sinPitch = max(-1.0f, min(1.0f, sinPitch));
```

### 4.5 清理 `gc` 校准流程

修改文件：`src/imu.h`

- 删除正式校准前重复的 `icm.getOffset(200)`；
- 直接打印当前保存的 offset；
- 只执行一次会先清零 offset 的 `calibrateICM()`；
- 校准后清空全局 `ypr`、`xyzReal`、上一帧数据和 IMU 异常状态；
- 将 `imuUpdated` 重置为 `false`，等待新任务产生真实数据。

### 4.6 修正校准后的启动顺序

修改文件：`src/reaction.h`

校准完成后的顺序改为：

1. 创建 IMU 任务；
2. 等待 3 秒，让任务持续采样并稳定滤波器；
3. 播放校准完成提示音；
4. 恢复主循环异常处理。

## 5. 涉及文件汇总

| 文件 | 修改内容 |
|---|---|
| `src/icm42670/petoi_icm42670p.h` | 声明 `resetFusion()`，增加时间基准和可重置 gyro bias 成员 |
| `src/icm42670/petoi_icm42670p.cpp` | 完整重置、首帧处理、`deltaT` 限制和数值保护 |
| `src/imu.h` | 校准后重置滤波器和全局状态，删除重复 offset 采样 |
| `src/reaction.h` | 先启动 IMU 任务，再进行 3 秒稳定等待 |

`OpenCatEsp32.ino` 保持使用：

```cpp
#define BITTLE
#define BiBoard_V1_0
```

## 6. 编译验证

使用以下配置完成编译：

```text
Board FQBN: esp32:esp32:esp32
ESP32 Arduino Core: 2.0.12
Partition Scheme: Huge APP (3MB No OTA/1MB SPIFFS)
```

编译命令：

```bash
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app \
  --output-dir build/BiBoard_V1_IMU_fix .
```

编译结果：

```text
Sketch uses 1761521 bytes (55%) of program storage space.
Maximum is 3145728 bytes.

Global variables use 70824 bytes (21%) of dynamic memory.
Leaving 256856 bytes for local variables.
Maximum is 327680 bytes.
```

生成的应用固件：

```text
build/BiBoard_V1_IMU_fix/OpenCatEsp32.ino.bin
```

应用固件 SHA-256：

```text
64EFE41AB4915840B045E2A27889F5267D20E2A8B2D32C1559952EC01CE6CD24
```

## 7. 烧录后验证步骤

1. 将机器人平放在稳定桌面上，启动时不要触碰机身。
2. 启动后查询或打印 IMU 数据，确认加速度 Z 轴约为 `9.8~10 m/s²`。
3. 静止状态下确认 pitch 和 roll 接近 `0°`，且不再周期性跳到 `±180°`。
4. 发送 `gc`，校准过程中保持机器人水平、静止。
5. 校准完成后继续观察至少 30 秒，确认任务恢复时没有立即出现 `Fall over`、`lifted` 或 `dropped`。
6. 轻微抬起、倾斜和推动机器人，确认姿态角方向及异常恢复动作正常。
7. 重新上电后重复静止测试，确认冷启动首帧同样稳定。

如果需要在排查期间避免误动作，可先用 `g?` 查看 gyro 状态；当 Balance 为 1 时，发送不带参数的 `g` 可临时关闭姿态反应。完成验证后再重新启用。

## 8. 预期结果

修复后应满足：

- 冷启动首帧不会出现由超大 `deltaT` 引起的姿态突变；
- `gc` 后不会继承校准前的四元数、时间戳或 bias；
- 水平静止时 pitch、roll 保持在合理的小角度范围；
- 不再因虚假的 `±180°` 姿态跳变持续执行恢复动作；
- 单次数值异常或长时间 I²C 停顿不会永久污染姿态滤波器。

## 9. 当前状态

- 源代码修复：完成
- BiBoard V1 固件编译：通过
- 编译产物生成：完成
- 实机烧录及长期运行验证：待执行

