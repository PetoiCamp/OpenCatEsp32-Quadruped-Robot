# 语音模块（Voice Module）修复记录

**日期：** 2026-05-27  
**涉及文件：** `src/OpenCat.h`、`src/voice.h`、`src/moduleManager.h`  
**适用板型：** BiBoard V1.0（Bittle X 等非 NYBBLE 产品）

---

## 背景问题

在非 NYBBLE 产品上默认同时启用 **Grove_Serial（S）**、**Voice（A）**、**BackTouch（B）** 后，出现以下现象：

1. 上电后语音模块无法控制机器狗，需在串口手动发送 `XAc`、`XAb` 后才正常。
2. USB 串口监视器看不到 Serial1 发出的指令（属正常现象，调试口为 USB `Serial`）。
3. 默认语言为英语时问题更明显；默认中文时相对正常。
4. 启动日志曾出现 `Reopen Voice Serial port` 及 `XAc` 被重复发送。

---

## 修改顺序总览

| 序号 | 文件 | 修改要点 |
|------|------|----------|
| 1 | `OpenCat.h` | 非 NYBBLE 默认模块激活 + 板型宏区分 |
| 2 | `voice.h` | 修复 `beginVoiceSerial()` 未真正 `begin` 的 Bug |
| 3 | `moduleManager.h` | Voice 优先初始化；Grove_Serial 切换保护 |
| 4 | `voice.h` | 抽取 `sendVoiceModuleCmd()`；新增 `voiceSyncAtStartup()` |
| 5 | `moduleManager.h` / `OpenCat.h` | 启动同步时机后移；初始化 `currentLan` |
| 6 | `voice.h` | 去掉误触发重开端口；调整指令顺序与延时 |
| 7 | `voice.h` | 英语默认专用启动序列 `XAc` → `XAb` → `XAa` |

---

## 1. 默认启用 Grove_Serial（按板型区分）

**文件：** `src/OpenCat.h`  
**位置：** `moduleActivatedQ[]` 默认值

### 修改内容

- **BiBoard_V1_0：** `{1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0}`  
  默认激活 **S（Grove_Serial）+ A（Voice）+ B（BackTouch）**  
  - Voice → Serial1（RX26 / TX25，9600）  
  - Grove → Serial2（RX9 / TX10，115200）  
  - 硬件上无 UART 冲突  

- **BiBoard V0.1/V0.2：** `{0, 1, ...}`  
  Voice 与 Grove 共用 Serial2，**不能**默认同时开启 Grove_Serial。

### 原因

注释已说明需默认开启 Grove 供 AI 对话，但原数组 `S=0`，与预期不符。

---

## 2. 修复 `beginVoiceSerial()` 未初始化 Serial1

**文件：** `src/voice.h`  
**函数：** `beginVoiceSerial()`

### 原问题

```cpp
if (!SERIAL_VOICE) {
  SERIAL_VOICE.begin(..., VOICE_RX, VOICE_TX);
}
```

在 ESP32 上 `HardwareSerial` 对象**恒为真**，`begin()` 带引脚参数的调用**永远不会执行**。  
Voice 实际未在 GPIO 25/26 上正确打开，而 Grove 已占用 Serial2（9/10）。

### 修改后

```cpp
SERIAL_VOICE.end();
SERIAL_VOICE.begin(SERIAL_VOICE_BAUD_RATE, SERIAL_8N1, VOICE_RX, VOICE_TX);  // BiBoard_V1_0
PTLF("Voice Serial1 @9600 (RX26 TX25)");
```

每次显式 `end()` 后按板型重新 `begin()`。

---

## 3. 模块初始化顺序与 Grove 保护

**文件：** `src/moduleManager.h`

### 3.1 `initModuleManager()` — Voice 优先

- 在循环初始化其他模块**之前**，若 Voice 已激活则先 `initModule(EXTENSION_VOICE)`。
- 保证 Serial1 在 Serial2（Grove）启动前完成引脚配置。

### 3.2 `reconfigureTheActiveModule()` — 保护 Grove_Serial

在 BiBoard_V1_0 下，使用 `X` 命令切换传感器模块时，除 **Voice、BackTouch** 外，**Grove_Serial** 也不再被误关闭：

```cpp
#ifdef BiBoard_V1_0
 || moduleList[i] == EXTENSION_GROVE_SERIAL
#endif
```

避免发 `XA...` 时关掉 AI 串口模块。

---

## 4. 统一串口发送与启动同步函数

**文件：** `src/voice.h`

### 4.1 新增 `sendVoiceModuleCmd()`

- 向语音模块发送 `X` + 命令（如 `Ac`、`Ab`、`Aa`）。
- 打印调试日志：`Voice TX: X...`。
- `set_voice()` 与 `voiceStop()` 均改为调用此函数。

### 4.2 新增 `voiceSyncAtStartup()`

- `voiceSetup()` 仅负责打开串口、统计自定义口令数量。
- **语言与 `XAc` 配置** 移至 `voiceSyncAtStartup()` 统一处理。

### 4.3 `set_voice()` 同步更新 `currentLan`

发送 `XAa` / `XAb` 时，同时写入 `defaultLan` 与 `currentLan` 到 Flash/EEPROM，便于下次上电恢复。

---

## 5. 启动同步时机后移

**文件：** `src/moduleManager.h`、`src/OpenCat.h`

### 修改

- 从 `initModuleManager()` **末尾移除** `voiceSyncAtStartup()` 调用。
- 改到 `initRobot()` 中 **`Ready!` 与 `beep()` 之后** 调用：

```cpp
#ifdef VOICE
  if (moduleActivatedQfunction(EXTENSION_VOICE))
    voiceSyncAtStartup();
#endif
```

### 原因

- IMU、BLE、舵机等初始化完成后再配置语音模块更稳定。
- 电池上电时语音模块 MCU 需要更长启动时间。

### 其它

**文件：** `src/OpenCat.h`

```cpp
char currentLan = 'a';  // 原为未显式初始化
```

---

## 6. 去掉误触发「重开端口」与指令顺序优化

**文件：** `src/voice.h`

### 6.1 删除「无应答则重发」

原逻辑在 50ms 内 `!SERIAL_VOICE.available()` 时会：

1. `SERIAL_VOICE.end()` + 重新 `begin()`
2. **再次发送同一条命令**（日志中表现为 `XAc` 发两次）
3. 可能打乱模块状态

### 修改后

- 新增 `drainVoiceRx()`：发送后等待约 200ms 排空 RX，**不**将无应答视为失败。
- 配置类指令通常无串口回显，属正常情况。

### 6.2 启动指令顺序（中间版本）

曾改为：`XAc` → 语言指令（`XAa` / `XAb`），指令间隔 350ms，前置 `delay(800)`。

---

## 7. 英语默认专用启动序列（最终方案）

**文件：** `src/voice.h`  
**函数：** `voiceSyncAtStartup()`

### 条件

Voice 模块已激活，且 **`defaultLan == 'a'`（默认英语）**。

### 发送顺序

| 步骤 | 指令 | 说明 |
|------|------|------|
| 1 | `XAc` | 打开语音动作响应 |
| 2 | `XAb` | 切换中文（模块冷启动需要） |
| 3 | `XAa` | 切回英语（最终工作语言） |

每条指令间隔 **350ms**，开始前 **`delay(800)`**。

### 默认中文（`defaultLan == 'b'`）

仍为：`XAc` → `XAb`（或按 `currentLan` 发送对应语言指令）。

### 预期启动日志（英语默认）

```
Ready!
Voice TX: XAc
Voice TX: XAb
Voice TX: XAa
Voice ready, language: English
Turn on the audio response
```

---

## 涉及文件一览

```
src/OpenCat.h          — moduleActivatedQ 默认值、currentLan 初始化、voiceSyncAtStartup 调用时机
src/voice.h            — 串口初始化、发送封装、启动同步、英语三指令序列
src/moduleManager.h    — Voice 优先 init、Grove_Serial 保护
```

---

## 测试建议

1. **BiBoard V1.0 + 电池上电**，确认无需手动 `XAc`/`XAb`。
2. 确认语音模块接线在 **Serial1：TX25 / RX26**，Grove/AI 在 **Serial2：TX10 / RX9**。
3. 英语默认：检查日志含 `Voice TX: XAc` → `XAb` → `XAa`。
4. 中文默认：日志应为 `Voice TX: XAc` → `XAb`，语言显示 Chinese。
5. 切换模块发 `XG` 等时，S、A、B 在 V1.0 上应保持激活（Grove 不被关）。

---

## 备注

- USB 串口监视器（`Serial`）**不会**显示 Serial1 发往语音模块的数据；仅能通过 `Voice TX:` 调试行确认。
- Flash 中 `moduleState` 会覆盖编译期默认 `moduleActivatedQ`；若行为异常可发 `X?` 查看模块状态或清除 NVS 后重烧。
- BiBoard V0.x 请勿默认同时开启 S 与 A（共用 Serial2）。
