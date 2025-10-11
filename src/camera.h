// #include <Wire.h>

#define MU_CAMERA
#define SENTRY1_CAMERA
#define GROVE_VISION_AI_V2
// #define TALL_TARGET

#ifdef SENTRY2_CAMERA
#undef MU_CAMERA
#undef SENTRY1_CAMERA
#include <Arduino.h>
#include <Sentry.h>

#define I2C_MODE
// #define SERIAL_MODE

#ifdef I2C_MODE
#include <Wire.h>
#define SENTRY2_ADDRESS 0x60
#endif

#ifdef SERIAL_MODE
#include <SoftwareSerial.h>
SoftwareSerial mySerial(UART_RX2, UART_TX2);
#endif
#endif

TaskHandle_t TASK_HandleCamera = NULL;
bool cameraTaskActiveQ = 0;

bool detectedObjectQ = false;
bool cameraReactionQ = true;
bool updateCoordinateLock = false;
int8_t cameraPrintQ = 0;
int xCoord = -1, yCoord = -1, width = 0, height = 0; // the x y returned by the sensor
int lastXcoord = -1, lastYcoord = -1;
int imgRangeX = 100;               // the frame size 0~100 on X and Y direction
int imgRangeY = 100;

extern TaskHandle_t TASK_imu;
extern bool updateGyroQ;
extern bool imuLockI2c;
extern bool cameraLockI2c;
extern bool gestureLockI2c;
extern bool eepromLockI2c;

// #ifdef BiBoard_V1_0
// #define USE_WIRE1  // use the Grove UART as the Wire1, which is independent of Wire used by the main devices, such as
                   // the gyroscope and EEPROM.
// #endif

// #ifdef USE_WIRE1
// #define CAMERA_WIRE Wire1
// #else
#define CAMERA_WIRE Wire
// #endif

#ifdef MU_CAMERA
// You need to install https://github.com/mu-opensource/MuVisionSensor3 as a zip library in Arduino IDE.
// Set the four dial switches on the camera as **v ^ v v** (the second switch dialed up to I2C) and connect the camera
// module to the I2C grove on NyBoard. The battery should be turned on to drive the servos.

// You can use these 3D printed structures to attach the camera module.
// https://github.com/PetoiCamp/NonCodeFiles/blob/master/stl/MuIntelligentCamera_mount.stl
// https://github.com/PetoiCamp/NonCodeFiles/blob/master/stl/bone.stl
// After uploading the code, you may need to press the reset buttons on the module and then the NyBoard.
/*
   Choose communication mode define here:
      I2C_MODE    : I2C mode, default pin: MU_SDA <==> ARDUINO_SDA, MU_SCL <==> ARDUINO_SCL
      SERIAL_MODE : Serial mode, default pin: MU_TX <==> ARDUINO_PIN3, MU_RX <==> ARDUINO_PIN2
*/
#define I2C_MODE
// #define SERIAL_MODE
/*
   Choose MU address here: 0x60, 0x61, 0x62, 0x63
          default address: 0x60
*/
#define MU_ADDRESS 0x50  // in later versions we set the I2C device to 0x50, 0x51, 0x52, 0x53
#define ALT_MU_ADDRESS 0x60

#include <MuVisionSensor.h>  //you need to download the library https://github.com/mu-opensource/MuVisionSensor3 into your ~/Documents/Arduino/libraries/
#ifdef I2C_MODE
#include <Wire.h>
#endif
#ifdef SERIAL_MODE
#include <SoftwareSerial.h>
SoftwareSerial mySerial(UART_RX2, UART_TX2);
#endif
#endif

#ifdef GROVE_VISION_AI_V2
#include "Seeed_Arduino_SSCMA/src/Seeed_Arduino_SSCMA.h"
// You need to install Seeed_Arduino_SSCMA via Arduino's library manager
// or download the library as a zip from https://github.com/Seeed-Studio/Seeed_Arduino_SSCMA
#endif

#define T_TUNER '>'
bool cameraSetupSuccessful = false;
int widthCounter;
int xDiff, yDiff;  // the scaled distance from the center of the frame
int currentX = 0, currentY = 0;  // the current x y of the camera's direction in the world coordinate

int8_t lensFactor, proportion, tranSpeed, pan, tilt, frontUpX, backUpX, frontDownX, backDownX, frontUpY, backUpY,
    frontDownY, backDownY, tiltBase, frontUp, backUp, frontDown, backDown;
int8_t sizePars;
#ifdef ROBOT_ARM
float adjustmentFactor = 1.5;
#else
float adjustmentFactor = 1;
#endif

#ifdef NYBBLE
int8_t nybblePars[] = {30, 11, 8, 10, 15, 60, -50, 31, -50, 45, -40, 40, -36, 0, 25, -60, 60, 16};
#else  // BITTLE or CUB
#ifdef MU_CAMERA
int8_t bittleMuPars[] = {30, 10, 8, 15, 15, 60, 80, 30, 80, 60, 30, 30, 70, 40, 40, 60, 40, -30};
#endif
#if defined GROVE_VISION_AI_V2
int8_t bittleGroveVisionPars[] = {20,
                                  20,
                                  8,
                                  10,
                                  12,
                                  int8_t(60 * adjustmentFactor),
                                  int8_t(75 * adjustmentFactor),
                                  int8_t(30 * adjustmentFactor),
                                  int8_t(75 * adjustmentFactor),
                                  20,
                                  25,
                                  10,
                                  25,
                                  0,
                                  30,
                                  60,
                                  40,
                                  -10};
#endif
#endif

int8_t *par[] = {&lensFactor, &proportion, &tranSpeed, &pan,      &tilt,      &frontUpX,
                 &backUpX,    &frontDownX, &backDownX, &frontUpY, &backUpY,   &frontDownY,
                 &backDownY,  &tiltBase,   &frontUp,   &backUp,   &frontDown, &backDown};

#ifdef MU_CAMERA
void muCameraSetup();
void read_MuCamera();
#endif

#ifdef SENTRY1_CAMERA
void sentry1CameraSetup();
void read_Sentry1Camera();
#endif

#ifdef SENTRY2_CAMERA
void sentry2CameraSetup();
void read_Sentry2Camera();
#endif

#ifdef GROVE_VISION_AI_V2
void groveVisionSetup();
void read_GroveVision();
#endif

int8_t *initPars;
bool cameraSetup() {
  if (!MuQ && !GroveVisionQ && !Sentry2Q) {
    return false;
  }

#ifdef NYBBLE
  initPars = nybblePars;
  sizePars = sizeof(nybblePars) / sizeof(int8_t);
#else  // BITTLE or CUB
#ifdef MU_CAMERA
  sizePars = sizeof(bittleMuPars) / sizeof(int8_t);
  if (MuQ) {
    initPars = bittleMuPars;
    imgRangeX = 100;  // the frame size 0~240 on X and Y direction
    imgRangeY = 100;
  }
#endif
#if defined GROVE_VISION_AI_V2 || defined SENTRY2_CAMERA
  sizePars = sizeof(bittleGroveVisionPars) / sizeof(int8_t);
  if (GroveVisionQ || Sentry2Q) {
    initPars = bittleGroveVisionPars;
    imgRangeX = 240;  // the frame size 0~240 on X and Y direction
    imgRangeY = 240;
  }
#endif
#endif
    
    // Acquire camera I2C lock and wait for other I2C operations to complete
#ifndef USE_WIRE1
    cameraLockI2c = true;  // Signal that camera wants to use I2C bus
while (
#ifdef GYRO_PIN
    imuLockI2c ||  // wait for the imu to release lock
#endif
    gestureLockI2c ||  // wait for the gesture to release lock
    eepromLockI2c)     // wait for the EEPROM operations to complete
  delay(1);
#endif

#ifdef USE_WIRE1
  CAMERA_WIRE.begin(UART_TX2, UART_RX2, 400000);
#endif
  for (byte i = 0; i < sizePars; i++)
    *par[i] = initPars[i];
  transformSpeed = 0;
  widthCounter = 0;
#ifdef MU_CAMERA
  if (MuQ)
    muCameraSetup();
#endif
#ifdef SENTRY1_CAMERA
  if (Sentry1Q) {
    lensFactor = 10;
    proportion = 20;
    sentry1CameraSetup();
  }
#endif
#ifdef GROVE_VISION_AI_V2
  if (GroveVisionQ) {
    groveVisionSetup();
  }
#endif
#ifdef SENTRY2_CAMERA
  if (Sentry2Q) {
    lensFactor = 10;
    proportion = 20;
    sentry2CameraSetup();
  }
#endif

#ifdef SHOW_FPS
  // used for FPS calculation
  fps = 0;
  loopTimer = millis();
#endif

//   Release camera I2C lock
 #ifndef USE_WIRE1
   cameraLockI2c = false;
 #endif
  return cameraSetupSuccessful;
}

void showRecognitionResult(int xCoord, int yCoord, int width, int height = -1) {
  if(!cameraTaskActiveQ) return;
  printToAllPorts(xCoord - imgRangeX / 2.0, false);  // get vision result: x axes value
  printToAllPorts('\t', false);
  printToAllPorts(yCoord - imgRangeY / 2.0, false);  // get vision result: y axes value
  printToAllPorts('\t', false);
  printToAllPorts("size = ", false);
  printToAllPorts(width, false);
  if (height >= 0) {
    printToAllPorts('\t', false);
    printToAllPorts(height, false);
  }
  printToAllPorts('\t', true);
}

// #define WALK  //let the robot move its body to follow people rather than sitting at the original position
// it works the best on the table so the robot doesn't need to loop upward.
// #define ROTATE

void cameraBehavior(int xCoord, int yCoord, int width) {
  if (cameraReactionQ) {
    while (updateCoordinateLock)
      ;
#ifdef WALK
    if (width > 45 && width != 52)  // 52 maybe a noise signal
      widthCounter++;
    else
      widthCounter = 0;
    if (width < 25 && width != 16) {  // 16 maybe a noise signal
      tQueue->addTask('k', currentX < -15 ? "wkR" : (currentX > 15 ? "wkL" : "wkF"),
                      (50 - width) * 50);  // walk towards you
      tQueue->addTask('k', "sit");
      tQueue->addTask('i', "");
      currentX = 0;
    } else if (widthCounter > 2) {
      tQueue->addTask('k', "bk", 1000);  // the robot will walk backward if you get too close!
      tQueue->addTask('k', "sit");
      tQueue->addTask('i', "");
      widthCounter = 0;
      currentX = 0;
    } else
#endif
    {
      xDiff =
          (xCoord
           - imgRangeX / 2.0);  // atan((xCoord - imgRangeX / 2.0) / (imgRangeX / 2.0)) * degPerRad;//almost the same
      yDiff = (yCoord - imgRangeY / 2.0);  // atan((yCoord - imgRangeY / 2.0) / (imgRangeX / 2.0)) * degPerRad;
      if (abs(xDiff) > 1 || abs(yDiff) > 1) {
        xDiff = xDiff / (lensFactor / 10.0);
        yDiff = yDiff / (lensFactor / 10.0);
        currentX = max(min(currentX - xDiff, 125), -125) / (proportion / 10.0);
        currentY = max(min(currentY - yDiff, 125), -125) / (proportion / 10.0);

        // PT('\t');
        // PT(currentX);
        // PT('\t');
        // PTL(currentY);

        // if (abs(currentX) < 60) {
        int8_t base[] = {0,       tiltBase, 0,      0,      0,         0,         0,        0,
                         frontUp, frontUp,  backUp, backUp, frontDown, frontDown, backDown, backDown};
        int8_t feedBackArray[][2] = {
            {pan, 0},
            {0, tilt},
            {0, 0},
            {0, 0},
            {0, 0},
            {0, 0},
            {0, 0},
            {0, 0},
            {frontUpX, (int8_t)-frontUpY},  // explicitly convert the calculation result to int8_t
            {(int8_t)-frontUpX, (int8_t)-frontUpY},
            {(int8_t)-backUpX, backUpY},
            {backUpX, backUpY},
            {(int8_t)-frontDownX, frontDownY},
            {frontDownX, frontDownY},
            {backDownX, (int8_t)-backDownY},
            {(int8_t)-backDownX, (int8_t)-backDownY},
        };
        transformSpeed = tranSpeed / 4.0;
        for (int i = 0; i < DOF; i++) {
          float adj = float(base[i]) + (feedBackArray[i][0] ? currentX * 10.0 / feedBackArray[i][0] : 0)
                      + (feedBackArray[i][1] ? currentY * 10.0 / feedBackArray[i][1] : 0);
          newCmd[i] = min(125, max(-125, int(adj)));
          // if (i == 0)//print adjustment of head pan joint
          // {
          //   PT(i);
          //   PT('\t');
          //   PT(adj);
          //   PT('\t');
          //   PT(int8_t(newCmd[i]));
          //   PTF(",\t");
          // }
        }
        // PTL();
        // newCmd[16] = '~';
        // printList((int8_t *)newCmd);
        transform((int8_t *)newCmd, 1, transformSpeed);
        token = '\0';  // avoid  conflicting with the balancing reaction
                       // }
#ifdef ROTATE
        else {
          tQueue->addTask('k', (currentX < 0 ? "vtR" : "vtL"), abs(currentX) * 40);  // spin its body to follow you
          tQueue->addTask('k', "sit");
          tQueue->addTask('i', "");
          currentX = 0;
        }
#endif
      }
    }
  }
}
int coords[3];

void taskReadCamera(void *par) {
  // PTHL("cameraTaskActiveQ = ", cameraTaskActiveQ);
  while (cameraTaskActiveQ) {
    // PTHL("imuLockI2c = ", imuLockI2c);
    // PTHL("gestureLockI2c = ", gestureLockI2c);
    // PTHL("eepromLockI2c = ", eepromLockI2c);
#ifndef USE_WIRE1
    while (
#ifdef GYRO_PIN
        imuLockI2c ||  // wait for the imu to release lock. potentially to cause dead lock with camera
#endif
        gestureLockI2c ||  // wait for the gesture to release lock. potentially to cause dead lock with camera
        eepromLockI2c)     // wait for the EEPROM operations to complete
      delay(1);
    cameraLockI2c = true;
#endif

#ifdef MU_CAMERA
    if (MuQ)
      read_MuCamera();
#endif
#ifdef SENTRY1_CAMERA
    if (Sentry1Q)
      read_Sentry1Camera();
#endif
#ifdef GROVE_VISION_AI_V2
    if (GroveVisionQ)
      read_GroveVision();
#endif
#ifdef SENTRY2_CAMERA
    if (Sentry2Q)
      read_Sentry2Camera();
#endif
    cameraLockI2c = false;

    // for checking the size of unused stack space
    // vTaskDelay(50 / portTICK_PERIOD_MS);  
    // Serial.println("Stack high water mark: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");
  }
  vTaskDelete(NULL);
}

void read_camera() {
  if (!cameraTaskActiveQ) {
    // Serial.print("Free heap: ");
    // Serial.println(ESP.getFreeHeap());
    PTLF("Create Camera Task...");
    BaseType_t result = xTaskCreatePinnedToCore(
      taskReadCamera,      // task function
      "TaskReadCamera",    // task name
      1800,                // task stack size​​
      NULL,                // parameters
      0,                   // priority
      &TASK_HandleCamera,  // handle
      0);                  // core
    if (result == pdPASS) {
      Serial.println("Camera task created successfully");
    } else {
      Serial.println("Failed to create camera task, error code: " + String(result));
    }
    cameraTaskActiveQ = 1;
    PTLF("Camera task activated.");
  }

  if (detectedObjectQ) {
    cameraBehavior(xCoord, yCoord, width);
    detectedObjectQ = false;
  }
}
#ifdef MU_CAMERA
MuVisionSensor *Mu;
int skip = 1;  //, counter; //an efforts to reduce motion frequency without using delay. set skip >1 to take effect
int i2cdelay = 3;
long noResultTime = 0;
MuVisionType object[] = {VISION_BODY_DETECT, VISION_BALL_DETECT};
String objectName[] = {"body", "ball"};
int objectIdx = 0;
int lastBallType;
void muCameraSetup() {
  PTL("Setup Mu3");
  uint8_t err;
  // initialized MU on the I2C port
  byte trial = 0;
  do {
    MuVisionSensor *Mu0 = new MuVisionSensor(MU_ADDRESS);
    err = Mu0->begin(&CAMERA_WIRE);
    if (err == MU_OK) {
      PTLF("MU initialized at 0x50");
      Mu = Mu0;
    } else {
      PTHL("Trial ", trial);
      if (!trial++) {  // only print once for the first time
        PTLF("Set the four dial switches on the camera as v ^ v v (the second switch dialed up to I2C)");
        PTLF("Then connect the camera to the I2C Grove socket with SDA and SCL pins!");
        PTLF("The battery should be turned on to drive the servos");
      }
      delete Mu0;
      MuVisionSensor *Mu1 = new MuVisionSensor(ALT_MU_ADDRESS);
      err = Mu1->begin(&CAMERA_WIRE);
      if (err == MU_OK) {
        PTLF("MU initialized at 0x60");
        Mu = Mu1;
      } else {
        delete Mu1;
      }
    }
  } while (err != MU_OK && trial < 1);
  //  shutServos();
  //  counter = 0;
  //  motion.loadBySkillName("rest");
  //  transform(motion.dutyAngles);
  cameraSetupSuccessful = (err == MU_OK);
  if (cameraSetupSuccessful)
    (*Mu).VisionBegin(object[objectIdx]);
  else
    PTL("Failed to initialize the camera!");
  noResultTime = millis();
}

void read_MuCamera() {
  if (cameraSetupSuccessful) {
    if ((*Mu).GetValue(object[objectIdx], kStatus)) {  // update vision result and get status, 0: undetected, other:
      // PTL(objectName[objectIdx]);
      updateCoordinateLock = true;
      xCoord = (int)(*Mu).GetValue(object[objectIdx], kXValue);
      yCoord = (int)(*Mu).GetValue(object[objectIdx], kYValue);
      width = (int)(*Mu).GetValue(object[objectIdx], kWidthValue);
      height = (int)(*Mu).GetValue(object[objectIdx], kHeightValue);
      updateCoordinateLock = false;
      // vvvvvvvvvvvv ball vvvvvvvvvvvvv
      if (objectIdx == 1) {
        int ballType = (*Mu).GetValue(object[objectIdx], kLabel);
        if (lastBallType != ballType) {
          switch ((*Mu).GetValue(object[objectIdx], kLabel)) {  // get vision result: label value
            case MU_BALL_TABLE_TENNIS:
              PTLF("table tennis");
              break;
            case MU_BALL_TENNIS:
              PTLF("tennis");
              break;
            default:
              PTLF("unknow ball type");
              break;
          }
          lastBallType = ballType;
        }
      }
      //^^^^^^^^^^^^^ ball ^^^^^^^^^^^^^^
      detectedObjectQ = true;
      noResultTime = millis();  // update the timer
    } else if (millis() - noResultTime > 2000) {  // if no object is detected for 2 seconds, switch object
      (*Mu).VisionEnd(object[objectIdx]);
      objectIdx = (objectIdx + 1) % (sizeof(object) / 2);
      (*Mu).VisionBegin(object[objectIdx]);
      PTL(objectName[objectIdx]);
      noResultTime = millis();
      beep(25, 50, 50, objectIdx + 1);
    }
  }
}
#endif

#ifdef SENTRY1_CAMERA

#define SENTRY_ADDR 0x60  // 0x61 0x62 0x63

void writeRegData(char reg_addr, char reg_data) {
  CAMERA_WIRE.beginTransmission(SENTRY_ADDR);
  CAMERA_WIRE.write(reg_addr);
  CAMERA_WIRE.write(reg_data);
  CAMERA_WIRE.endTransmission();
  delay(1);
}

char readRegData(char reg_addr) {
  CAMERA_WIRE.beginTransmission(SENTRY_ADDR);
  CAMERA_WIRE.write(reg_addr);  // read label
  CAMERA_WIRE.endTransmission();

  CAMERA_WIRE.requestFrom(SENTRY_ADDR, 1);  // request 1 byte from slave device
  char ret = 0;
  while (CAMERA_WIRE.available()) {
    ret = CAMERA_WIRE.read();  // receive a byte
  }
  return ret;
  delay(1);
}

void sentry1CameraSetup() {
  // CAMERA_WIRE.begin();  // join i2c bus (address optional for master)
  delay(2000);  // wait for sentry1 startup, not necessary
  PTLF("Setup Sentry1");
  writeRegData(0x20, 0x07);  // set vision id: 7 (body for Sentry1)
  writeRegData(0x21, 0x01);  // enable vision
  // writeRegData(0x22, 0x10);  // set vision level: 0x10=Sensitive/Speed 0x20=balance(default if not set) 0x30=accurate
  // ..........[UPDATE]
  delay(1000);
  PTLF("Sentry1 ready");
  lensFactor = 10;  // default value is 30 ..........[UPDATE]
  proportion = 30;  // default value is 20 ..........[UPDATE]
  pan = 20;  // default value is 15 ..........[UPDATE]
  tranSpeed = 1;
  cameraSetupSuccessful = true;
}

char frame_cnt = 0;

void read_Sentry1Camera() {
  //  Serial.println("loop...");
  if (cameraSetupSuccessful) {
    char frame_new = readRegData(0x1F);  // update frame count ..........[UPDATE]
    if (frame_new == frame_cnt) {  // if frame is unchanged, delay some time and return ..........[UPDATE]
      delay(10);  // ..........[UPDATE]
      return;  //  ..........[UPDATE]
    }  //  ..........[UPDATE]
    frame_cnt = frame_new;  // update frame_cnt ..........[UPDATE]

    char label = readRegData(0x89);  // read label/ value Low Byte, 1=detected, 0=undetected
    //  Serial.print("label: ");
    //  Serial.println(label);//
    if (label == 1) {  // have detected
      updateCoordinateLock = true;
      xCoord = readRegData(0x81);  // read x value Low Byte, 0~100, (LB is enought for sentry 1)
      yCoord = readRegData(0x83);  // read y value Low Byte, 0~100
      width = readRegData(0x85);  // read width value Low Byte, 0~100
      height = readRegData(0x87);  // read height value Low Byte, 0~100, not necessary if already have read width
                                   // do something ......
      updateCoordinateLock = false;
      detectedObjectQ = true;
      delay(10);
    }
  }
  // do something or delay some time
}
#endif

#ifdef SENTRY2_CAMERA
/* Define the Sentry model. Here use Sentry2 */
typedef Sentry2 Sentry;

/* Instantiate Sentry, create a Sentry variable, and specify the Sentry address */
#define VISION_TYPE Sentry::kVisionFace
Sentry sentry(SENTRY2_ADDRESS);
void sentry2CameraSetup() {
  sentry_err_t err = SENTRY_OK;
  PTL("Waiting for sentry initialize...");
#ifdef I2C_MODE
  // CAMERA_WIRE.begin();  // join i2c bus (address optional for master)
  /* Use I2C to initialize Sentry. If err returns 0, the initialization is normal, otherwise the corresponding error
   * code is returned. */
  while (SENTRY_OK != sentry.begin(&CAMERA_WIRE)) {
    yield();
  }
#endif  // I2C_MODE
#ifdef SERIAL_MODE
  mySerial.begin(9600);
  while (SENTRY_OK != sentry.begin(&mySerial)) {
    yield();
  }
#endif  // SERIAL_MODE
  PTL("Sentry begin Success.");
  err = sentry.VisionBegin(VISION_TYPE);
  PT("sentry.VisionBegin(kVisionFace) ");
  if (err) {
    PT("Error: 0x");
  } else {
    PT("Success: 0x");
    cameraSetupSuccessful = true;
  }
  PTD(err, HEX);
  PTL();
}

void read_Sentry2Camera() {
  int obj_num = sentry.GetValue(VISION_TYPE, kStatus);
  if (obj_num >= 1) {
    // Serial.print("Totally ");
    // Serial.print(obj_num);
    // Serial.println(" objects");
    updateCoordinateLock = true;
    xCoord = sentry.GetValue(VISION_TYPE, kXValue, 1);  // read x value
    yCoord = sentry.GetValue(VISION_TYPE, kYValue, 1);  // read y value
    width = sentry.GetValue(VISION_TYPE, kWidthValue, 1);  // read width value
    height = sentry.GetValue(VISION_TYPE, kHeightValue, 1);  // read height value
    updateCoordinateLock = false;
    detectedObjectQ = true;
    // for (int i = 1; i <= obj_num; ++i) {
    //   int x = sentry.GetValue(VISION_TYPE, kXValue, i);
    //   int y = sentry.GetValue(VISION_TYPE, kYValue, i);
    //   int w = sentry.GetValue(VISION_TYPE, kWidthValue, i);
    //   int h = sentry.GetValue(VISION_TYPE, kHeightValue, i);
    //   int l = sentry.GetValue(VISION_TYPE, kLabel, i);
    //   Serial.print("  obj");
    //   Serial.print(i);
    //   Serial.print(": ");
    //   Serial.print("x=");
    //   Serial.print(x);
    //   Serial.print(",y=");
    //   Serial.print(y);
    //   Serial.print(",w=");
    //   Serial.print(w);
    //   Serial.print(",h=");
    //   Serial.print(h);
    //   Serial.print(",label=");
    //   Serial.println(l);
    // }
  }
}
#endif

#ifdef GROVE_VISION_AI_V2
SSCMA AI;

// OPT_ANGLE values
enum OptAngle : uint16_t {
  OPT_ANGLE_0 = 0,
  OPT_ANGLE_90 = 0xffff & (1 << 12),
  OPT_ANGLE_180 = 0xffff & (1 << 13),
  OPT_ANGLE_270 = 0xffff & (1 << 14),
};

// OPT_DETAIL values
enum OptResolution : uint8_t {
  OPT_DETAIL_240 = 0,  // 240*240 Auto
  OPT_DETAIL_480,  // 480*480 Auto
  OPT_DETAIL_640,  // 640*480 Auto
};
void groveVisionSetup() {
  PTLF("Setup Grove Vision AI Module");
  // CAMERA_WIRE.begin(10, 9, 400000);
  
  // End the TASK_imu task when activating Grove Vision AI V2
#ifdef GYRO_PIN
  if (TASK_imu != NULL) {
      while (eTaskGetState(TASK_imu) != eDeleted) {
          PTHL("task state:", eTaskGetState(TASK_imu));
          if(eTaskGetState(TASK_imu)== eReady){
              TASK_imu = NULL;
              break;
          }
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }
    // PTHL("Initial task state:", eTaskGetState(TASK_imu));
    // Ensure release all locks that may block IMU task
  }
#endif
  // Adjust I2C clock frequency, reduce to improve stability
  CAMERA_WIRE.setClock(100000); // Reduce to 100kHz, more stable than default 400kHz
  
  // Wait for a while before initializing AI module to ensure I2C bus stability
  delay(500); // Increase delay to 500ms
  
  // Try AI module initialization
  Serial.println("Initializing Grove Vision AI Module...");
  bool aiInitSuccess = false;
  
  // Method 1: Try standard initialization
  if (AI.begin(&CAMERA_WIRE)) {
    aiInitSuccess = true;
    Serial.println("Standard AI.begin() succeeded!");
  } else {
    Serial.println("Standard AI.begin() failed, using manual method...");
    
    // Method 2: Manual initialization (more reliable under Bluetooth interference)
    AI.set_rx_buffer(8192);  // 8KB receive buffer
    AI.set_tx_buffer(1024);  // 1KB transmit buffer
    
    // Manually set sensor parameters
    uint8_t result = AI.setSensor(true, OPT_DETAIL_240 + OPT_ANGLE_90);
    if (result == CMD_OK) {
      aiInitSuccess = true;
      Serial.println("Manual initialization succeeded!");
    } else {
      Serial.print("Manual initialization failed with code: 0x");
      Serial.println(result, HEX);
    }
  }
  
  if (!aiInitSuccess) {
    Serial.println("AI module initialization failed!");
    Serial.println("Please check hardware connection and power supply");
    return;
  }

  uint8_t count = 0;
  bool sensorEnable = true;
  uint16_t sensorVal =
      OPT_DETAIL_240
      + (strcmp(MODEL, "Bittle X+Arm") ? OPT_ANGLE_90 : OPT_ANGLE_0);  // 240*240, rotate 90 degrees if Bittle R

  Serial.println("Set sensor angle and resolution...");
  do {
    delay(10);
    if (CMD_OK == AI.setSensor(sensorEnable, sensorVal)) {
      cameraSetupSuccessful = true;
      break;
    }
    count++;
    PTHL("count:", count);
  } while (count < 3);
}

void read_GroveVision() {
  // PTHL("cameraSetupSuccessful = ", cameraSetupSuccessful);
  // PTHL("AI.invoke() = ", AI.invoke());
  if (cameraSetupSuccessful && !AI.invoke()) {
    if (AI.boxes().size() >= 1) {
      updateCoordinateLock = true;
      xCoord = AI.boxes()[0].x;  // read x value
      yCoord = AI.boxes()[0].y;  // read y value
      width = AI.boxes()[0].w;  // read width value
      height = AI.boxes()[0].h;  // read height value
      updateCoordinateLock = false;
      detectedObjectQ = true;
      // if (cameraPrintQ)
      // {
      //   for (int i = 0; i < AI.boxes().size(); i++)
      //   {
      //     Serial.print("Box[");
      //     Serial.print(i);
      //     Serial.print("] target=");
      //     Serial.print(AI.boxes()[i].target);
      //     Serial.print(", score=");
      //     Serial.print(AI.boxes()[i].score);
      //     Serial.print(", x=");
      //     Serial.print(AI.boxes()[i].x);
      //     Serial.print(", y=");
      //     Serial.print(AI.boxes()[i].y);
      //     Serial.print(", w=");
      //     Serial.print(AI.boxes()[i].w);
      //     Serial.print(", h=");
      //     Serial.println(AI.boxes()[i].h);
      //   }
      // }
#ifdef SHOW_FPS
      if (cameraPrintQ == 2 && cameraTaskActiveQ) 
      {
        FPS();
      } 
#endif
    }
  }
}
#endif
