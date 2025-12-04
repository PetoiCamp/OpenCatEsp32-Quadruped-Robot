// Petoi Voice Command Module
// Doc: https://docs.petoi.com/extensible-modules/voice-command-module
// use the software serial port on the NyBoard to read the module. connect the module to the grove socket with pin 6 and 7.
// or use the serial 2 port on the BiBoard to read the module. connect the module to the pin Tx2 and Rx2.
// if you wire the module up with the USB programmer directly, connect the module's Tx to the programmer's Rx, and Rx to Tx.
// Rongzhong Li
// Petoi LLC
// Jan 12, 2023

#define SERIAL_VOICE_BAUD_RATE 9600
#define MAX_CUSTOMIZED_CMD 10

// Speak "start learning" to record your voice commands in order. You can record up to 10 voice commands
// Speak "stop learning" to stop in the middle
// Speak one of the recorded voice commands to trigger the reaction
// Speak "clear the learning data" to delete all the recordings at once. (you cannot delete a specific recording)
// The reactions below are already defined in the program. You may use SkillComposer to design new skills then import them into InstinctX.h
// Other serial commands are also supported, such as joint movements and melody

// 说”开始学习“开始录音，最多10条
// 说”结束学习“停止录入
// 说出口令触发反应
// 说”清除数据“删除所有自定义口令（无法删除单条口令）
// 下列行为是程序中预设的，您可以用技能创作坊设计新技能并导入到 InstinctX.h
// 支持其他的串口指令，比如活动关节和旋律

// #define VOICE_MODULE_SAMPLE
String customizedCmdList[] = {
  // "rg",
  "fl",  // learn skill with feedback servos
  "fr",  // replay skill learned with feedback servos
  "fF",  // movement follower demo with feedback servos
#ifdef BITTLE
#ifdef ROBOT_ARM
  "kpickF",                          // pick front 捡起来
  "kputD",                           // put down 放下
  "khuntL",                          // hunt 捕猎
  "kshowOff",                        // show off 展示
  "kputL",                           // put left 收起来
  "ktossL",                          // toss left 左抛
  "klaunchL",                        // launch 发射
  "kclapL",                          // clap 鼓掌
  "ktossF",                          // toss front 前抛
  "qc-2:0>kclap:1000>kpickF:1000>",  // calibrate arm (for QA) 校准(工厂用)
#else
  "kpu1",                                                                  // single-handed pushups
  "m0 80 0 -80 0 0",                                                       // wave head
  "kmw",                                                                   // moonwalk
  "b14,8,14,8,21,8,21,8,23,8,23,8,21,4,19,8,19,8,18,8,18,8,16,8,16,8,14,4,\
  21,8,21,8,19,8,19,8,18,8,18,8,16,4,21,8,21,8,19,8,19,8,18,8,18,8,16,4,\
  14,8,14,8,21,8,21,8,23,8,23,8,21,4,19,8,19,8,18,8,18,8,16,8,16,8,14,4",  // twinkle star
  "T",                                                                     // call the last skill data sent by the Skill Composer
  "6th",
  "7th",
  "8th",
  "9th",
  "10th"  // define up to 10 customized commands.
#endif
#else//if defined NYBBLE
  "kluckyL",   // lucky cat 招财猫
  "kwsfL",  // lick paws 舔爪子
  "qksit:100>i0 20 1 0 8 -70 12 0 15 10:0>o1 0, 0 40 -20 4 0, 1 -30 20 4 30, 8 -70 10 4 60, 12 -10 10 4 0, 15 10 0 4 0:100>m0 0 1 -20 2 0:0>ksit:0",
  // "kwsfL",                                                                 //wash face 洗脸
  "khuntL",                                                                // hunt 捕猎
  "m0 80 0 -80 0 0",                                                       // wave head                                                                //
  "b14,8,14,8,21,8,21,8,23,8,23,8,21,4,19,8,19,8,18,8,18,8,16,8,16,8,14,4,\
  21,8,21,8,19,8,19,8,18,8,18,8,16,4,21,8,21,8,19,8,19,8,18,8,18,8,16,4,\
  14,8,14,8,21,8,21,8,23,8,23,8,21,4,19,8,19,8,18,8,18,8,16,8,16,8,14,4",  // twinkle star
  "T",                                                                     // repeat
  "xl",                                                                    // learn a new trick 学习动作
  "xp",                                                                    // play the trick 表演动作
  "10th"                                                                   // define up to 10 customized commands.
#endif
};
int listLength = 0;
bool enableVoiceQ = true;
void beginVoiceSerial() {
  if (!SERIAL_VOICE) {
    // PTL("Begin Voice Serial port");
#ifdef BiBoard_V1_0
    SERIAL_VOICE.begin(SERIAL_VOICE_BAUD_RATE, SERIAL_8N1, VOICE_RX, VOICE_TX);
#else
    SERIAL_VOICE.begin(SERIAL_VOICE_BAUD_RATE);
#endif
    SERIAL_VOICE.setTimeout(5);
  }
  delay(20);
}

void set_voice(char *cmd) {  // send some control command directly to the module
  // XAa: switch English
  // XAb: switch Chinese
  // XAc: turn on the sound response
  // XAd: turn off the sound response
  // XAe: start learning
  // XAf: stop learning
  // XAg: clear the learning data
  if (cmd[1] == 'a' || cmd[1] == 'b') {  // enter "XAa" in the serial monitor or add button "X65,97" in the mobile app to switch to English
    // 在串口监视器输入指令“XAa”或在手机app创建按键"X65,97"来切换到英文
    defaultLan = cmd[1];
#ifdef I2C_EEPROM_ADDRESS
    i2c_eeprom_write_byte(EEPROM_DEFAULT_LAN, defaultLan);
    i2c_eeprom_write_byte(EEPROM_CURRENT_LAN, currentLan);
#else
    config.putChar("defaultLan", defaultLan);
    config.putChar("currentLan", currentLan);
#endif
    PTHL("Default language: ", defaultLan == 'b' ? " Chinese" : " English");
  }
  byte c = 0;
  while (cmd[c] != '\0' && cmd[c] != '~')
    c++;
  cmd[c] = '\0';
  SERIAL_VOICE.print("X");
  SERIAL_VOICE.println(cmd);
  delay(10);
  if (!SERIAL_VOICE.available()) {  // the serial port may need to re-open for the first time after rebooting. Don't know why.
    SERIAL_VOICE.end();
    PTLF("Reopen Voice Serial port");
    beginVoiceSerial();
    delay(10);
    SERIAL_VOICE.print("X");
    SERIAL_VOICE.println(cmd);
    delay(10);
  }
  while (SERIAL_VOICE.available())  // avoid echo
    PT(char(SERIAL_VOICE.read()));
  PTL();
  if (!strcmp(cmd, "Ac"))  // enter "XAc" in the serial monitor or add button "X65,99" in the mobile app to enable voice reactions
                           // 在串口监视器输入指令“XAc”或在手机app创建按键"X65,99"来激活语音动作
    enableVoiceQ = true;
  else if (!strcmp(cmd, "Ad"))  // enter "XAd" in the serial monitor or add button "X65,100" in the mobile app to disable voice reactions
                                // 在串口监视器输入指令“XAd”或在手机app创建按键"X65,100"来禁用语音动作
    enableVoiceQ = false;


  printToAllPorts('X');  // the blue read runs on a separate core.
  // if the message arrives after the reaction(), it may not reply 'X' to BLE and the mobile app will keep waiting for it.
  resetCmd();
}

void voiceSetup() {
  PTLF("Init voice");
  listLength = min(int(sizeof(customizedCmdList) / sizeof(customizedCmdList[0])), MAX_CUSTOMIZED_CMD);
  PTF("Number of customized voice commands on the main board: ");
  PTL(listLength);
  beginVoiceSerial();
  if (currentLan != defaultLan) {
    char temp[4] = "XA\0";
    temp[2] = defaultLan;
    SERIAL_VOICE.println(temp);
    currentLan = defaultLan;
#ifdef I2C_EEPROM_ADDRESS
    i2c_eeprom_write_byte(EEPROM_CURRENT_LAN, currentLan);
#else
    config.putChar("currentLan", currentLan);
#endif
  }

  SERIAL_VOICE.println("XAc");
  PTLF("Turn on the audio response");
  enableVoiceQ = true;
}
void voiceStop() {
  beginVoiceSerial();
  SERIAL_VOICE.println("XAd");
  delay(5);
  SERIAL_VOICE.end();
  PTLF("Turn off the audio response");
  enableVoiceQ = false;
}

void read_voice() {
  if (SERIAL_VOICE.available()) {
    String raw = SERIAL_VOICE.readStringUntil('\n');
#ifdef BT_CLIENT
    if (btConnected) {
      PTLF("Ignore voice for remote controllor");
      return;
    }
#endif
    PTL(raw);
    byte index = (byte)raw[2];  // interpret the 3rd byte as integer
    int shift = -1;
    if (index > 10 && index < 61) {
      if (index < 21) {  // 11 ~ 20 are customized commands, and their indexes should be shifted by 11
        index -= 11;
        PT(index);
        PT(' ');
        if (index < listLength) {
          raw = customizedCmdList[index];
          token = raw[0];
          shift = 1;
        } else {
          PTLF("Undefined!");
        }
      } else if (index < 61) {  // 21 ~ 60 are preset commands, and their indexes should be shifted by 21.
                                // But we don't need to use their indexes.
#ifdef VOICE_MODULE_SAMPLE
        token = T_SKILL;
        shift = 3;
#else
        token = raw[3];
        shift = 4;
#endif
      }
      if (enableVoiceQ) {
        const char *cmd = raw.c_str() + shift;
        tQueue->addTask(token, shift > 0 ? cmd : "", 2500);
        if (strlen(cmd) > 0) {
          char end = cmd[strlen(cmd) - 1];
          if (!strcmp(cmd, "bk") || !strcmp(cmd, "x") || (end >= 'A' && end <= 'Z')) {
            tQueue->addTask('k', "up");
          }
        }
      }
    } else {
      switch (tolower(index)) {
        case 'a':  // say "Bing-bing" to switch English /说“冰冰”切换英文
          {
            PTLF("Switch English");
            currentLan = 'a';
#ifdef I2C_EEPROM_ADDRESS
            i2c_eeprom_write_byte(EEPROM_CURRENT_LAN, 'a');
#else
            config.putChar("currentLan", 'a');
#endif
            if (lastToken == 'c') {  // only change the default language in calibration mode.
                                     //otherwise the language will roll back to default after reboot
              defaultLan = 'a';
#ifdef I2C_EEPROM_ADDRESS
              i2c_eeprom_write_byte(EEPROM_DEFAULT_LAN, 'a');
#else
              config.putChar("defaultLan", 'a');
#endif
              PTHL("Default language: ", defaultLan == 'b' ? " Chinese" : " English");
            }
            break;
          }
        case 'b':  // say "Di-di" to switch Chinese /说“滴滴”切换中文
          {
            PTLF("Switch Chinese");
            currentLan = 'b';
#ifdef I2C_EEPROM_ADDRESS
            i2c_eeprom_write_byte(EEPROM_CURRENT_LAN, 'b');
#else
            config.putChar("currentLan", 'b');
#endif
            if (lastToken == 'c') {
              defaultLan = 'b';
#ifdef I2C_EEPROM_ADDRESS
              i2c_eeprom_write_byte(EEPROM_DEFAULT_LAN, 'b');
#else
              config.putChar("defaultLan", 'b');
#endif
              PTHL("Default language: ", defaultLan == 'b' ? " Chinese" : " English");
            }
            break;
          }
        case 'c':  // say "play sound" to enable voice reactions / 说“打开音效”激活语音动作
          {
            enableVoiceQ = true;
            PTLF("Turn on the audio response");
            break;
          }
        case 'd':  // say "be quiet" to disable voice reactions / 说“安静点”禁用语音动作
          {
            enableVoiceQ = false;
            PTLF("Turn off the audio response");
            break;
          }
        case 'e':
          {
            PTLF("Start learning");
            break;
          }
        case 'f':
          {
            PTLF("Stop learning");
            break;
          }
        case 'g':
          {
            PTLF("Delete all learning data!");
            break;
          }
      }
    }
  }
}
