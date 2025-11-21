#ifdef VOICE
#include "voice.h"
#endif

#ifdef VOICE_LD3320
#include "voiceLD3320.h"
#endif

#ifdef CAMERA
#include "camera.h"
#endif

#ifdef ULTRASONIC
#include "ultrasonic.h"
#endif

#ifdef GESTURE
#include "gesture.h"
#endif

#ifdef PIR
#include "pir.h"
#endif

#ifdef DOUBLE_TOUCH
#include "doubleTouch.h"
#endif

#ifdef DOUBLE_LIGHT
#include "doubleLight.h"
#endif

#ifdef DOUBLE_INFRARED_DISTANCE
#include "doubleInfraredDistance.h"
#endif

#ifdef BACKTOUCH_PIN
#include "backTouch.h"
#endif

#ifdef ROBOT_ARM
#include "robotArm.h"
#endif

#ifdef OTHER_MODULES
#endif

#ifdef ALL_RANDOM
#else
// #define GYRO_PIN 0
#endif

int8_t indexOfModule(char moduleName) {
  for (byte i = 0; i < sizeof(moduleList) / sizeof(char); i++)
    if (moduleName == moduleList[i])
      return i;
  return -1;
}
bool moduleActivatedQfunction(char moduleCode) {
  return moduleActivatedQ[indexOfModule(moduleCode)];
}

int8_t activeModuleIdx() {  // designed to work if only one active module is allowed
  for (byte i = 0; i < sizeof(moduleList) / sizeof(char); i++)
    if (moduleActivatedQ[i])
      return i;
  return -1;
}

void initModule(char moduleCode) {
  bool successQ = true;
  int8_t index = indexOfModule(moduleCode);
  switch (moduleCode) {
    case EXTENSION_GROVE_SERIAL:
      {
        PTLF("Start Serial2");
#ifdef BiBoard_V1_0
        Serial2.begin(115200, SERIAL_8N1, 9, 10);
#else
        Serial2.begin(115200, SERIAL_8N1, UART_RX2, UART_TX2);
#endif
        Serial2.setTimeout(SERIAL_TIMEOUT);
        break;
      }
#ifdef VOICE
    case EXTENSION_VOICE:
      {
        voiceSetup();
        break;
      }
#endif
#ifdef ULTRASONIC
    case EXTENSION_ULTRASONIC:
      {
        loadBySkillName("sit");
        rgbUltrasonicSetup();
        break;
      }
#endif
#ifdef DOUBLE_TOUCH
    case EXTENSION_DOUBLE_TOUCH:
      {
        loadBySkillName("sit");
        touchSetup();
        break;
      }
#endif
#ifdef DOUBLE_LIGHT
    case EXTENSION_DOUBLE_LIGHT:
      {
        loadBySkillName("sit");
        doubleLightSetup();
        break;
      }
#endif
#ifdef DOUBLE_INFRARED_DISTANCE
    case EXTENSION_DOUBLE_IR_DISTANCE:
      {
        loadBySkillName("sit");
        doubleInfraredDistanceSetup();
        break;
      }
#endif
#ifdef PIR
    case EXTENSION_PIR:
      {
        pirSetup();
        break;
      }
#endif
#ifdef GESTURE
    case EXTENSION_GESTURE:
      {
        loadBySkillName("sit");
        gestureSetup();
        break;
      }
#endif
#ifdef BACKTOUCH_PIN
    case EXTENSION_BACKTOUCH:
      {
        // Detect at init whether the sensor is connected; disable if not
        bool connected = true;
// #ifdef BACKTOUCH_PIN
        pinMode(BACKTOUCH_PIN, INPUT_PULLDOWN); // avoid floating
        analogRead(BACKTOUCH_PIN);              // prime ADC channel
        delayMicroseconds(80);                  // settle
        int raw = analogRead(BACKTOUCH_PIN);
        PTHL("BackTouch raw", raw);
        // With sensor connected and no touch it should read 4095. Otherwise treat as not connected.
        if (raw != 4095) {
          connected = false;
        }
// #endif
        if (connected) {
          backTouchSetup();
        } else {
          successQ = false; // 不启用该模块
        }
        break;
      }
#endif
#ifdef CAMERA
    case EXTENSION_CAMERA:
      {
        PTLF("Setting updateGyroQ to false...");
        updateGyroQ = false;
        i2cDetect(Wire);
// #if defined BiBoard_V1_0 && !defined NYBBLE
//         i2cDetect(Wire1);
// #endif
        loadBySkillName("sit");
        if (!cameraSetup()) {
          int i = indexOfModule(moduleCode);
          PTHL("*** Fail to start ", moduleNames[i]);
          successQ = false;
        }
        break;
      }
#endif
#ifdef QUICK_DEMO
    case EXTENSION_QUICK_DEMO:
      {
        break;
      }
#endif
  }
  moduleActivatedQ[index] = successQ;
}

void stopModule(char moduleCode) {
  switch (moduleCode) {
    case EXTENSION_GROVE_SERIAL:
      {
        Serial2.end();
        PTL("Stop Serial 2");
        break;
      }
#ifdef VOICE
    case EXTENSION_VOICE:
      {
        voiceStop();
        break;
      }
#endif
#ifdef ULTRASONIC
    case EXTENSION_ULTRASONIC:
      {
        // ultrasonicStop();   // Todo
        ultrasonicLEDinitializedQ = false;
        break;
      }
#endif
#ifdef DOUBLE_TOUCH
    case EXTENSION_DOUBLE_TOUCH:
      {
        break;
      }
#endif
#ifdef DOUBLE_LIGHT
    case EXTENSION_DOUBLE_LIGHT:
      {
        break;
      }
#endif
#ifdef DOUBLE_INFRARED_DISTANCE
    case EXTENSION_DOUBLE_IR_DISTANCE:
      {
        manualHeadQ = false;
        break;
      }
#endif
#ifdef PIR
    case EXTENSION_PIR:
      {
        break;
      }
#endif
#ifdef GESTURE
    case EXTENSION_GESTURE:
      {
        gestureStop();
        break;
      }
#endif
#ifdef CAMERA
    case EXTENSION_CAMERA:
      {
        // cameraStop();   // Todo
        cameraSetupSuccessful = false;
        cameraTaskActiveQ = 0;
        break;
      }
#endif
#ifdef BACKTOUCH_PIN
    case EXTENSION_BACKTOUCH:
      {
        break;
      }
#endif
#ifdef QUICK_DEMO
    case EXTENSION_QUICK_DEMO:
      {
        break;
      }
#endif
  }
}
void showModuleStatus() {
  byte moduleCount = sizeof(moduleList) / sizeof(char);
  printListWithoutString((char *)moduleList, moduleCount);
  printListWithoutString(moduleActivatedQ, moduleCount);
  moduleDemoQ = (moduleActivatedQfunction(EXTENSION_DOUBLE_LIGHT)
                 || moduleActivatedQfunction(EXTENSION_DOUBLE_TOUCH)
                 || moduleActivatedQfunction(EXTENSION_GESTURE)
                 || moduleActivatedQfunction(EXTENSION_DOUBLE_IR_DISTANCE)
                 || moduleActivatedQfunction(EXTENSION_CAMERA)
                 || moduleActivatedQfunction(EXTENSION_PIR)
                 // || moduleActivatedQfunction(EXTENSION_BACKTOUCH)
                 // || moduleActivatedQfunction(EXTENSION_ULTRASONIC)
                 || moduleActivatedQfunction(EXTENSION_QUICK_DEMO));
}

void reconfigureTheActiveModule(char *moduleCode) {
  if (moduleCode[0] == '?') {
    showModuleStatus();
    return;
  }
  bool statusChangedQ = false;
  
  // Determine target module and operation type based on the elegant original design
  char targetModule = moduleCode[0];
  bool isCloseOnlyOperation = false;
  
  // Handle 'X' or 'X~' -> close all
  if (strlen(moduleCode) == 0 || (strlen(moduleCode) >= 1 && (moduleCode[0] == '~' || moduleCode[0] == '\n'))) {
    targetModule = '-'; // Use '-' to represent close all, avoiding confusion
  }
  // Handle 'Xc' -> close specific module (lowercase)
  else if (strlen(moduleCode) >= 1 && islower(moduleCode[0])) {
    targetModule = toupper(moduleCode[0]); // Convert to uppercase for module identification
    isCloseOnlyOperation = true; // Only close this specific module, don't enable anything
  }
  // Handle 'XC' -> enable specific module (uppercase) - use original logic
  
  // PTHL("mode", moduleCode);                                          // negative number will deactivate all the modules
  for (byte i = 0; i < sizeof(moduleList) / sizeof(char); i++) {                                               // disable unneeded modules
    if (!moduleActivatedQ[i]) continue;
    
    // For lowercase: only process target module; For original logic: only process non-target modules
    if ((isCloseOnlyOperation && moduleList[i] != targetModule) || 
        (!isCloseOnlyOperation && moduleList[i] == targetModule)) continue;
    
    // Original logic: protect voice and backtouch unless closing all
    if (!isCloseOnlyOperation && 
        (moduleList[i] == EXTENSION_VOICE || moduleList[i] == EXTENSION_BACKTOUCH) && 
        targetModule != '-') continue;
    
    // Unified disable logic
    PTHL("- disable", moduleNames[i]);
    stopModule(moduleList[i]);
    moduleActivatedQ[i] = false;
    statusChangedQ = true;
#ifdef I2C_EEPROM_ADDRESS
    i2c_eeprom_write_byte(EEPROM_MODULE_ENABLED_LIST + i, false);
#endif
  }

  // Original logic: enable target module (skip for close-only operations)
  if (!isCloseOnlyOperation) {
    for (byte i = 0; i < sizeof(moduleList) / sizeof(char); i++) {
      if (moduleList[i] == targetModule && !moduleActivatedQ[i]) {
        PTHL("+  enable", moduleNames[i]);
        moduleActivatedQ[i] = true;
#ifdef I2C_EEPROM_ADDRESS
        i2c_eeprom_write_byte(EEPROM_MODULE_ENABLED_LIST + i, true);
#endif
        statusChangedQ = true;
      }
    }
  }

  if (statusChangedQ){  // if the status of the modules has changed, show the new status
#ifndef I2C_EEPROM_ADDRESS
    config.putBytes("moduleState", moduleActivatedQ, sizeof(moduleList) / sizeof(char));
#endif
    showModuleStatus();
    for (byte i = 0; i < sizeof(moduleList) / sizeof(char); i++) {
      if (moduleList[i] == targetModule && moduleActivatedQ[i]) {
        initModule(moduleList[i]);
      }
    }
  }
}

void initModuleManager() {
  byte moduleCount = sizeof(moduleList) / sizeof(char);
  PTHL("Module count: ", moduleCount);
  for (byte i = 0; i < moduleCount; i++) {
    if (moduleActivatedQ[i]) {
      initModule(moduleList[i]);
    }
#ifdef VOICE
    else if (moduleList[i] == EXTENSION_VOICE) {
      voiceStop();
    }
#endif
#if defined ULTRASONIC && defined NYBBLE
    else if (moduleList[i] == EXTENSION_ULTRASONIC) {
      rgbUltrasonicSetup();
    }
#endif
  }
  showModuleStatus();
}

void read_serial() {
  Stream *serialPort = NULL;
// String source;
#ifdef BT_SSP
  if (SerialBT.available()) {  // give BT a higher priority over wired serial
    serialPort = &SerialBT;
    // source = "BT";
  } else
#endif
    if (moduleActivatedQ[0] && Serial2.available()) {
    serialPort = &Serial2;
  } else if (Serial.available()) {
    serialPort = &Serial;
    // source = "SER";
  }
  if (serialPort) {
    token = serialPort->read();
    lowerToken = tolower(token);
    newCmdIdx = 2;
    delay(1);                                                  // leave enough time for serial read
    terminator = (token >= 'A' && token <= 'Z') ? '~' : '\n';  // capitalized tokens use binary encoding for long data commands
                                                               //'~' ASCII code = 126; may introduce bug when the angle is 126 so only use angles <= 125
    serialTimeout = (token == T_SKILL_DATA || lowerToken == T_BEEP) ? SERIAL_TIMEOUT_LONG : SERIAL_TIMEOUT;
    lastSerialTime = millis();
    do {
      if (serialPort->available()) {
        // long current = millis();
        // PTH(source, current - lastSerialTime);
        do {
          if (((token == T_SKILL || lowerToken == T_INDEXED_SIMULTANEOUS_ASC || lowerToken == T_INDEXED_SEQUENTIAL_ASC) && cmdLen >= spaceAfterStoringData) || cmdLen > BUFF_LEN) {
            PTH("Cmd Length: ", cmdLen);
            PTF("OVF");
            beep(5, 100, 50, 5);
            do {
              serialPort->read();
            } while (serialPort->available());
            printToAllPorts(token);
            token = T_SKILL;
            strcpy(newCmd, "up");
            cmdLen = 2;
            return;
          }
          newCmd[cmdLen++] = serialPort->read();
          // PTHL(newCmd[cmdLen - 1], int8_t(newCmd[cmdLen - 1]));
        } while (serialPort->available());
        lastSerialTime = millis();
      }
    } while (newCmd[cmdLen - 1] != terminator && long(millis() - lastSerialTime) < serialTimeout);  // the lower case tokens are encoded in ASCII and can be entered in Arduino IDE's serial monitor
                                                                                                    // if the terminator of the command is set to "no line ending" or "new line", parsing can be different
                                                                                                    // so it needs a timeout for the no line ending case
    // PTH("* " + source, long(millis() - lastSerialTime));
    if (!(token >= 'A' && token <= 'Z') || token == 'X' || token == 'R' || token == 'W') {  // serial monitor is used to send lower cased tokens by users
                                                                                            // delete the unexpected '\r' '\n' if the serial monitor sends line ending symbols
      leftTrimSpaces(newCmd, &cmdLen);                                                      // allow space between token and parameters, such as "k sit"
      for (int i = cmdLen - 1; i >= 0; i--) {                                               // delete the '/r' and '/n' if the serial monitor is configured to send terminators
        if ((newCmd[i] == '\n') || (newCmd[i] == '\r')) {
          newCmd[i] = '\0';
          cmdLen--;
        }
      }
    }
    cmdLen = (newCmd[cmdLen - 1] == terminator) ? cmdLen - 1 : cmdLen;
    newCmd[cmdLen] = (token >= 'A' && token <= 'Z') ? '~' : '\0';
    if (token >= 'A' && token <= 'Z')
      newCmd[cmdLen + 1] = '\0';
    newCmdIdx = 2;
    // PTH("read_serial, cmdLen = ", cmdLen);
    // printCmdByType(token, newCmd);
  }
}

void readSignal() {
  moduleIndex = activeModuleIdx();
#ifdef IR_PIN
  read_infrared();  //  newCmdIdx = 1
#endif
  read_serial();  //  newCmdIdx = 2
  
  // Smart Bluetooth mode reading
#if defined(BT_BLE)
  if (activeBtMode == BT_MODE_SERVER) {    // activeBtMode == BT_MODE_NONE
    detectBle();  //  newCmdIdx = 3;
    readBle();
  }
#endif
#if defined(BT_CLIENT)
  if (activeBtMode == BT_MODE_CLIENT) {    // activeBtMode == BT_MODE_NONE
    readBleClient();
  }
#endif
// #ifdef WEB_SERVER
//   if (webServerConnected)
//     webServer.handleClient();
// #endif
#ifdef VOICE
  if (moduleActivatedQ[indexOfModule(EXTENSION_VOICE)])
    read_voice();
#endif

  long current = millis();
  if (newCmdIdx)
    idleTimer = millis() +
#ifdef DOUBLE_INFRARED_DISTANCE
                0
#else
                IDLE_TIME
#endif
      ;
  else if (token != T_SERVO_CALIBRATE && token != T_SERVO_FOLLOW && token != T_SERVO_FEEDBACK && current - idleTimer > 0) {
    if (moduleIndex == -1)  // no active module
      return;
#ifdef CAMERA
    if (moduleActivatedQ[indexOfModule(EXTENSION_CAMERA)])
      read_camera();
#endif
#ifdef ULTRASONIC
    if (moduleActivatedQ[indexOfModule(EXTENSION_ULTRASONIC)])
      read_RGBultrasonic();
#endif
#ifdef GESTURE
    if (moduleActivatedQ[indexOfModule(EXTENSION_GESTURE)])
    {
      gestureGetValue = read_gesture();
      // PTHL("gestureValue02:", gestureGetValue);
    }
#endif
#ifdef PIR
    if (moduleActivatedQ[indexOfModule(EXTENSION_PIR)])
      read_PIR();
#endif
#ifdef DOUBLE_TOUCH
    if (moduleActivatedQ[indexOfModule(EXTENSION_DOUBLE_TOUCH)])
      read_doubleTouch();
#endif
#ifdef DOUBLE_LIGHT
    if (moduleActivatedQ[indexOfModule(EXTENSION_DOUBLE_LIGHT)])
      read_doubleLight();
#endif
#ifdef DOUBLE_INFRARED_DISTANCE
    if (moduleActivatedQ[indexOfModule(EXTENSION_DOUBLE_IR_DISTANCE)])
      read_doubleInfraredDistance();  // has some bugs
#endif
#ifdef BACKTOUCH_PIN
    if (moduleActivatedQ[indexOfModule(EXTENSION_BACKTOUCH)])
      read_backTouch();
#endif
    // powerSaver -> 4
    // other -> 5
    // randomMind -> 100
    if (autoSwitch) {
      randomMind();             // make the robot do random demos
      powerSaver(POWER_SAVER);  // make the robot rest after a certain period, the unit is seconds
    }
  }
}

// — read human sensors (top level) —
void readHuman() {
#ifdef TOUCH0
  read_touch();
#endif
}
// — generate behavior by fusing all sensors and instruction
String decision() {
  return "";
}

void read_sound() {
}

void read_GPS() {
}
#ifdef TOUCH0
void read_touch() {
  byte touchPin[] = {
    TOUCH0,
    TOUCH1,
    TOUCH2,
    TOUCH3,
  };
  for (byte t = 0; t < 4; t++) {
    int touchValue = touchRead(touchPin[t]);  // do something with the touch?
    //    PT(touchValue);
    //    PT('\t');
  }
  //  PTL();
}
#endif
void readEnvironment() {
#ifdef GYRO_PIN
  // if (updateGyroQ && !(frame % imuSkip))
  //   imuUpdated = readIMU();
  if (updateGyroQ)
    if (imuUpdated && printGyroQ)
      print6Axis();
#endif
  read_sound();
  read_GPS();
}
