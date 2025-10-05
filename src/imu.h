#ifdef GYRO_PIN
#include "mpu6050/src/I2Cdev.h"
#include "mpu6050/src/MPU6050_6Axis_MotionApps_V6_12.h"

bool calibrateQ = false;
float ypr[3];
float previous_ypr[3];

// Turning control variables
bool turningQ = false;           // Turning control switch
float targetYawAngle = 0.0;     // Target angle to reach
float initialYawAngle = 0.0;    // Initial angle when turning started
bool needTurning = false;        // Flag to prevent turning exception from being skipped

#define IMU_SKIP 1
#define IMU_SKIP_MORE 23  // use prime number to avoid repeatly skipping the same joint
#define ARX xyzReal[0]
#define ARY xyzReal[1]
#define ARZ xyzReal[2]
#define AWX aaWorld.x
#define AWY aaWorld.y
#define AWZ aaWorld.z
#define GRAVITY 10.0
#define IMU_PERIOD 5
float gFactor = GRAVITY / 8192;
byte imuSkip = IMU_SKIP;
float previousXYZ[3];
int8_t yprTilt[3];
float xyzReal[3];
int thresX, thresY, thresZ;
byte imuBad2[] = {20, 12, 18, 12, 16, 12, 16, 16, 16, 16, 16, 8};  // fail during calibration
#ifdef IMU_MPU6050

// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class using DMP (MotionApps v6.12)
// 6/21/2012 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//      2019-07-10 - Uses the new version of the DMP Firmware V6.12
//                 - Note: I believe the Teapot demo is broken with this versin as
//                 - the fifo buffer structure has changed
//      2016-04-18 - Eliminated a potential infinite loop
//      2013-05-08 - added seamless Fastwire support
//                 - added note about gyro calibration
//      2012-06-21 - added note about Arduino 1.0.1 + Leonardo compatibility error
//      2012-06-20 - improved FIFO overflow handling and simplified read process
//      2012-06-19 - completely rearranged DMP initialization code and simplification
//      2012-06-13 - pull gyro and accel data from FIFO packet instead of reading directly
//      2012-06-09 - fix broken FIFO read sequence and change interrupt detection to RISING
//      2012-06-05 - add gravity-compensated initial reference frame acceleration output
//                 - add 3D math helper file to DMP6 example sketch
//                 - add Euler output and Yaw/Pitch/Roll output formats
//      2012-06-04 - remove accel offset clearing for better results (thanks Sungon Lee)
//      2012-06-01 - fixed gyro sensitivity to be 2000 deg/sec instead of 250
//      2012-05-30 - basic DMP initialization working

/* ============================================
  I2Cdev device library code is placed under the MIT license
  Copyright (c) 2012 Jeff Rowberg

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  ===============================================
*/

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "mpu6050/src/I2Cdev.h"

#include "mpu6050/src/MPU6050_6Axis_MotionApps_V6_12.h"
// #include "MPU6050.h" // not necessary if using MotionApps include file

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

/* =========================================================================
   NOTE: In addition to connection 3.3v, GND, SDA, and SCL, this sketch
   depends on the MPU-6050's INT pin being connected to the Arduino's
   external interrupt #0 pin. On the Arduino Uno and Mega 2560, this is
   digital I/O pin 2.
   ========================================================================= */

/* =========================================================================
   NOTE: Arduino v1.0.1 with the Leonardo board generates a compile error
   when using Serial.write(buf, len). The Teapot output uses this method.
   The solution requires a modification to the Arduino USBAPI.h file, which
   is fortunately simple, but annoying. This will be fixed in the next IDE
   release. For more info, see these links:

   http://arduino.cc/forum/index.php/topic,109987.0.html
   http://code.google.com/p/arduino/issues/detail?id=958
   ========================================================================= */

// uncomment "OUTPUT_READABLE_QUATERNION" if you want to see the actual
// quaternion components in a [w, x, y, z] format (not best for parsing
// on a remote host such as Processing or something though)
// #define OUTPUT_READABLE_QUATERNION

// uncomment "OUTPUT_READABLE_EULER" if you want to see Euler angles
// (in degrees) calculated from the quaternions coming from the FIFO.
// Note that Euler angles suffer from gimbal lock (for more info, see
// http://en.wikipedia.org/wiki/Gimbal_lock)
// #define OUTPUT_READABLE_EULER

// uncomment "OUTPUT_READABLE_YAWPITCHROLL" if you want to see the yaw/
// pitch/roll angles (in degrees) calculated from the quaternions coming
// from the FIFO. Note this also requires gravity vector calculations.
// Also note that yaw/pitch/roll angles suffer from gimbal lock (for
// more info, see: http://en.wikipedia.org/wiki/Gimbal_lock)
#define OUTPUT_READABLE_YAWPITCHROLL

// uncomment "OUTPUT_READABLE_REALACCEL" if you want to see acceleration
// components with gravity removed. This acceleration reference frame is
// not compensated for orientation, so +X is always +X according to the
// sensor, just without the effects of gravity. If you want acceleration
// compensated for orientation, us OUTPUT_READABLE_WORLDACCEL instead.
// #define OUTPUT_READABLE_REALACCEL
/* REALACCEL represents the acceleration related to the sensor.
   Even if the robot is not moving, it will be large if the sensor is tilted.
   Because gravity has a component in that tilted direction.
   It's useful to use aaReal.z to determine if the sensor is flipped up-side-down.
*/

// uncomment "OUTPUT_READABLE_WORLDACCEL" if you want to see acceleration
// components with gravity removed and adjusted for the world frame of
// reference (yaw is relative to initial orientation, since no magnetometer
// is present in this case). Could be quite handy in some cases.
#define OUTPUT_READABLE_WORLDACCEL
/* WORLDACCEL represents the acceleration related to the world reference.
   It will be close to zero as long as the robot is not moved.
   It's useful to detect the wobbling about the sensor's original position.
*/

// uncomment "OUTPUT_TEAPOT" if you want output that matches the
// format used for the InvenSense teapot demo
// #define OUTPUT_TEAPOT
class mpu6050p : public MPU6050 {
  // MPU control/status vars
  bool dmpReady = false;   // set true if DMP init was successful
  uint8_t mpuIntStatus;    // holds actual interrupt status byte from MPU
  uint8_t devStatus;       // return status after each device operation (0 = success, !0 = error)
  uint16_t packetSize;     // expected DMP packet size (default is 42 bytes)
  uint16_t fifoCount;      // count of all bytes currently in FIFO
  uint8_t fifoBuffer[64];  // FIFO storage buffer

  // orientation/motion vars
  Quaternion q;         // [w, x, y, z]         quaternion container
  VectorInt16 aa;       // [x, y, z]            accel sensor measurements
  VectorInt16 gy;       // [x, y, z]            gyro sensor measurements
  VectorFloat gravity;  // [x, y, z]            gravity vector
  float euler[3];       // [psi, theta, phi]    Euler angle container
public:
  VectorInt16 aaReal;   // [x, y, z]            gravity-free accel sensor measurements
  VectorInt16 aaWorld;  // [x, y, z]            world-frame accel sensor measurements
  int16_t *xyzReal[3] = {&aaReal.x, &aaReal.y, &aaReal.z};
  float ypr[3];     // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector. unit is radian
  float a_real[3];  // [x, y, z]            gravity vector in the real world

  // packet structure for InvenSense teapot demo
  uint8_t teapotPacket[14] = {'$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n'};

  // ================================================================
  // ===               INTERRUPT DETECTION ROUTINE                ===
  // ================================================================

  // volatile bool mpuInterrupt = false;  // indicates whether MPU interrupt pin has gone high
  // void dmpDataReady() {
  //   mpuInterrupt = true;
  // }

  // The REALACCEL numbers are calculated with respect to the orientation of the sensor itself, so that if it is flat
  // and you move it straight up, the "Z" accel will change, but if you flip it up on one side and move it in the new
  // relative "up" direction (along the sensor's Z axis), it will still register acceleration on the Z axis.
  // Essentially, it is sensor-oriented acceleration which removes the effects of gravity and any non-flat/level
  // orientation.

  // The WORLDACCEL numbers are calculated to ignore orientation. Moving it straight up while flat will look the same as
  // the REALACCEL numbers, but if you then flip it upside-down and do the exact same movement ("up" with respect to
  // you), you'll get exactly the same numbers as before, even though the sensor itself is upside-down.
  void calibrateMPU() {
    PTL("MPU6050 calibration started");
    
    // wait for other I2C devices to be idle
    while (imuLockI2c || gestureLockI2c) delay(1);
    
    PTLF("Calibrate MPU6050...");
    CalibrateAccel(20);
    CalibrateGyro(20);
    
#ifdef I2C_EEPROM_ADDRESS
    PTL("Writing MPU6050 calibration data...");
    
    // exclusive mode for writing calibration data
    eepromLockI2c = true;
    
    i2c_eeprom_write_int16(EEPROM_MPU, getXAccelOffset());
    i2c_eeprom_write_int16(EEPROM_MPU + 2, getYAccelOffset());
    i2c_eeprom_write_int16(EEPROM_MPU + 4, getZAccelOffset());
    i2c_eeprom_write_int16(EEPROM_MPU + 6, getXGyroOffset());
    i2c_eeprom_write_int16(EEPROM_MPU + 8, getYGyroOffset());
    i2c_eeprom_write_int16(EEPROM_MPU + 10, getZGyroOffset());
    
    eepromLockI2c = false;
#else
    config.putShort("mpu0", getXAccelOffset());
    config.putShort("mpu1", getYAccelOffset());
    config.putShort("mpu2", getZAccelOffset());
    config.putShort("mpu3", getXGyroOffset());
    config.putShort("mpu4", getYGyroOffset());
    config.putShort("mpu5", getZGyroOffset());
#endif
    PrintActiveOffsets();
  }
  bool read_mpu6050() {
    if (!dmpReady)
      return false;
    if (dmpGetCurrentFIFOPacket(fifoBuffer)) {  // Get the Latest packet
      // display Euler angles in degrees
      dmpGetQuaternion(&q, fifoBuffer);
      dmpGetAccel(&aa, fifoBuffer);
      dmpGetEuler(euler, &q);
      dmpGetGravity(&gravity, &q);
      dmpGetYawPitchRoll(ypr, &q, &gravity);
      dmpGetLinearAccel(&aaReal, &aa, &gravity);
      dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);

      for (byte i = 0; i < 3; i++) {  // no need to flip yaw
        ypr[i] *= degPerRad;
        a_real[i] = *xyzReal[i] / 8192.0 * GRAVITY;
#ifdef BiBoard_V0_1  // # rotate 180 degree
        ypr[i] = -ypr[i];
        if (i != 2)
          a_real[i] = -a_real[i];
#endif
      }
      return true;
    }
    return false;
  }

  void mpu6050Setup(bool calibrateQ = true) {
    // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.setClock(400000);  // 400kHz I2C clock. Comment this line if having compilation difficulties
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
#endif
        // initialize serial communication
    // (115200 chosen because it is required for Teapot Demo output, but it's
    // really up to you depending on your project)
    // Serial.begin(115200);
    // while (!Serial)
    // ;  // wait for Leonardo enumeration, others continue immediately

    // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3V or Arduino
    // Pro Mini running at 3.3V, cannot handle this baud rate reliably due to
    // the baud timing being too misaligned with processor ticks. You must use
    // 38400 or slower in these cases, or use some kind of external separate
    // crystal solution for the UART timer.
    int connectAttempt = 0;
    do {
      // initialize device
      PTLF("\nInitializing MPU6050...");
#if defined CONFIG_DISABLE_HAL_LOCKS && CONFIG_DISABLE_HAL_LOCKS == 1
      PTL("OK");
      PTL("If the program stucks, reinstall Arduino ESP32 boards version 2.0.12. Newer version may cause bugs!");
#else
      PTL("If the program stucks, modify the header file:\n  "
          "https://docs.petoi.com/arduino-ide/upload-sketch-for-biboard#sdkconfig.h");
#endif
      initialize();
      // pinMode(INTERRUPT_PIN, INPUT);
      // verify connection
      PTF("- Testing MPU connections...attempt ");
      PTL(connectAttempt++);
      delay(500);
    } while (!testConnection());
    PTLF("- MPU6050 connection successful");

    // load and configure the DMP
    PTLF("- Initializing DMP...");

    devStatus = dmpInitialize();
    PT("MPU offsets: ");
    for (byte m = 0; m < 6; m++) {
#ifdef I2C_EEPROM_ADDRESS
      mpuOffset[m] = i2c_eeprom_read_int16(EEPROM_MPU + m * 2);
#else
      char key[8];
      sprintf(key, "mpu%d", m);
      // short takes two bytes on ESP32
      mpuOffset[m] = (int16_t)config.getShort(key);
#endif
      PTT(mpuOffset[m], '\t');
    }
    PTL();
    // supply the gyro offsets here, scaled for min sensitivity
    setXAccelOffset(mpuOffset[0]);
    setYAccelOffset(mpuOffset[1]);
    setZAccelOffset(mpuOffset[2]);  // gravity
    setXGyroOffset(mpuOffset[3]);  // yaw
    setYGyroOffset(mpuOffset[4]);  // pitch
    setZGyroOffset(mpuOffset[5]);  // roll

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
      // Calibration Time: generate offsets and calibrate our MPU6050
      if (calibrateQ) {
        calibrateMPU();
      }
      // turn on the DMP, now that it's ready
      PTLF("- Enabling DMP...");
      setDMPEnabled(true);

      // enable Arduino interrupt detection
      // PTF("- Enabling interrupt detection (Arduino external interrupt ");
      // PT(digitalPinToInterrupt(INTERRUPT_PIN));
      // PTLF(")...");
      // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
      mpuIntStatus = getIntStatus();

      // set our DMP Ready flag so the main loop() function knows it's okay to use it
      PTLF("- DMP ready! Waiting for the first interrupt...");
      dmpReady = true;

      // get expected DMP packet size for later comparison
      packetSize = dmpGetFIFOPacketSize();
    } else {
      // ERROR!
      // 1 = initial memory load failed
      // 2 = DMP configuration updates failed
      // (if it's going to break, usually the code will be 1)
      PTF("- DMP Initialization failed (code ");
      PT(devStatus);
      PTLF(")");
    }

    delay(10);
    read_mpu6050();
  }
  void print6AxisMacro() {
#ifdef OUTPUT_READABLE_QUATERNION
    // display quaternion values in easy matrix form: w x y z
    PT("quat\t");
    PT(q.w);
    PT("\t");
    PT(q.x);
    PT("\t");
    PT(q.y);
    PT("\t");
    PT(q.z);
    PT("\t");
#endif

#ifdef OUTPUT_READABLE_EULER
    // display Euler angles in degrees
    PT("euler\t");
    PT(euler[0] * 180 / M_PI);
    PT("\t");
    PT(euler[1] * 180 / M_PI);
    PT("\t");
    PT(euler[2] * 180 / M_PI);
    PT("\t");
#endif

#ifdef OUTPUT_READABLE_YAWPITCHROLL
    // display angles in degrees
    PT("ypr\t");
    PT(ypr[0]);
    PT("\t");
    PT(ypr[1]);
    PT("\t");
    PT(ypr[2]);
    PT("\t");
    /*
      dmpGetAccel(&aa, fifoBuffer);
      PT("\tRaw Accl XYZ\t");
      PT(aa.x);
      PT("\t");
      PT(aa.y);
      PT("\t");
      PT(aa.z);
      dmpGetGyro(&gy, fifoBuffer);
      PT("\tRaw Gyro XYZ\t");
      PT(gy.x);
      PT("\t");
      PT(gy.y);
      PT("\t");
      PT(gy.z);
      PT("\t");
    */
#endif

#ifdef OUTPUT_READABLE_WORLDACCEL
    // display initial world-frame acceleration, adjusted to remove gravity
    // and rotated based on known orientation from quaternion
    PT("aworld\t");
    PT(aaWorld.x);
    PT("\t");
    PT(aaWorld.y);
    PT("\t");
    PT(aaWorld.z);
    PT("\t");
#endif

#ifdef OUTPUT_READABLE_REALACCEL
    // display real acceleration, adjusted to remove gravity
    PT("areal\t");
    PT(aaReal.x);
    PT("\t");
    PT(aaReal.y);
    PT("\t");
#endif
    PT("areal.z\t");
    PT(aaReal.z);  // becomes negative when flipped
    PT("\t");

    PTL();
  }
};

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
mpu6050p mpu;
// MPU6050 mpu(0x69); // <-- use for AD0 high
#endif

#ifdef IMU_ICM42670
#include "icm42670/petoi_icm42670p.h"

imu42670p icm(Wire, 1);



void calibrateICM() {
  PTL("ICM42670 calibration started");
  
  // wait for other I2C devices to be idle
  while (imuLockI2c || gestureLockI2c) delay(1);
  
  PTLF("Calibrate ICM42670...");
  for (byte i = 0; i < 3; i++) {
    icm.offset_accel[i] = 0;
    icm.offset_gyro[i] = 0;
  }
  icm.getOffset(200);
  if (icm.offset_gyro[0] == -32768 || icm.offset_gyro[1] == -32768 || icm.offset_gyro[2] == -32768) {
    PT("Reading error: ");
    PT(icm.offset_gyro[0]);
    PT('\t');
    PT(icm.offset_gyro[1]);
    PT('\t');
    PT(icm.offset_gyro[2]);
    PTL('\t');
    while (!Serial.available()) {
      playMelody(imuBad2, sizeof(imuBad2) / 2);
    }
    while (Serial.available())
      Serial.read();
  }
  
#ifdef I2C_EEPROM_ADDRESS
  PTL("Writing ICM42670 calibration data...");
  
  // exclusive mode for writing calibration data
  eepromLockI2c = true;
  
  i2c_eeprom_write_float(EEPROM_ICM, icm.offset_accel[0]);
  i2c_eeprom_write_float(EEPROM_ICM + 4, icm.offset_accel[1]);
  i2c_eeprom_write_float(EEPROM_ICM + 8, icm.offset_accel[2]);
  i2c_eeprom_write_float(EEPROM_ICM + 12, icm.offset_gyro[0]);
  i2c_eeprom_write_float(EEPROM_ICM + 16, icm.offset_gyro[1]);
  i2c_eeprom_write_float(EEPROM_ICM + 20, icm.offset_gyro[2]);
  
  eepromLockI2c = false;
#else
  config.putFloat("icm_accel0", icm.offset_accel[0]);
  config.putFloat("icm_accel1", icm.offset_accel[1]);
  config.putFloat("icm_accel2", icm.offset_accel[2]);
  config.putFloat("icm_gyro0", icm.offset_gyro[0]);
  config.putFloat("icm_gyro1", icm.offset_gyro[1]);
  config.putFloat("icm_gyro2", icm.offset_gyro[2]);
#endif
  PT("New ICM offsets: ");
  for (byte i = 0; i < 3; i++)
    PTT(icm.offset_accel[i], '\t');
  for (byte i = 0; i < 3; i++)
    PTT(icm.offset_gyro[i], '\t');
  PTL();
}
void icm42670Setup(bool calibrateQ = true) {
  PTLF("\nInitializing ICM42670...");
  icm.begin();
  icm.init(200, 2, 250);
  // Wait icm to start
  delay(10);

  bool hasCalibData = false;
#ifdef I2C_EEPROM_ADDRESS
  // Check if EEPROM has valid calibration data (non-zero values)
  float testValue = i2c_eeprom_read_float(EEPROM_ICM);
  if (testValue != 0.0 && !isnan(testValue)) {
    icm.offset_accel[0] = i2c_eeprom_read_float(EEPROM_ICM);
    icm.offset_accel[1] = i2c_eeprom_read_float(EEPROM_ICM + 4);
    icm.offset_accel[2] = i2c_eeprom_read_float(EEPROM_ICM + 8);
    icm.offset_gyro[0] = i2c_eeprom_read_float(EEPROM_ICM + 12);
    icm.offset_gyro[1] = i2c_eeprom_read_float(EEPROM_ICM + 16);
    icm.offset_gyro[2] = i2c_eeprom_read_float(EEPROM_ICM + 20);
    hasCalibData = true;
  }
#else
  if (config.isKey("icm_accel0")) {
    icm.offset_accel[0] = config.getFloat("icm_accel0");
    icm.offset_accel[1] = config.getFloat("icm_accel1");
    icm.offset_accel[2] = config.getFloat("icm_accel2");
    icm.offset_gyro[0] = config.getFloat("icm_gyro0");
    icm.offset_gyro[1] = config.getFloat("icm_gyro1");
    icm.offset_gyro[2] = config.getFloat("icm_gyro2");
    hasCalibData = true;
  }
#endif

  if (hasCalibData) {
    PT("Using ICM offsets: ");
    for (byte i = 0; i < 3; i++)
      PTT(icm.offset_accel[i], '\t');
    for (byte i = 0; i < 3; i++)
      PTT(icm.offset_gyro[i], '\t');
    PTL();
  } else
    PTLF("Calibrate for the first time!");

  // Calibration Time: generate offsets and calibrate our MPU6050
  if (calibrateQ) {
    calibrateICM();
  } else {
    Serial.println("calibration already done");
  }
  PTL();
}
#endif

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

#define PRINT_ACCELERATION
void print6Axis() {
  if (!updateGyroQ)
    return;
  char buffer[50];  // Adjust buffer size as needed
#ifdef IMU_ICM42670
  if (icmQ) {
#ifdef PRINT_ACCELERATION
    sprintf(buffer, "ICM:%6.2f%6.2f%6.2f%7.1f%7.1f%7.1f\t",  //
            icm.a_real[0] * GRAVITY, icm.a_real[1] * GRAVITY, icm.a_real[2] * GRAVITY, -icm.ypr[0], icm.ypr[1], icm.ypr[2]);
#else
    sprintf(buffer, "ICM%7.1f%7.1f%7.1f\t", -icm.ypr[0], icm.ypr[1], icm.ypr[2]);
#endif
    printToAllPorts(buffer, 0);
  }
#endif
#ifdef IMU_MPU6050
  if (mpuQ) {
#ifdef PRINT_ACCELERATION
    sprintf(buffer, "MCU:%6.2f%6.2f%6.2f%7.1f%7.1f%7.1f",  // 7x6 = 42
            mpu.a_real[0], mpu.a_real[1], mpu.a_real[2], -mpu.ypr[0], mpu.ypr[1], mpu.ypr[2]);  //, aaWorld.z);
#else
    sprintf(buffer, "MCU%7.1f%7.1f%7.1f", -mpu.ypr[0], mpu.ypr[1], mpu.ypr[2]);
#endif
    printToAllPorts(buffer, 0);
  }
#endif
  PTL();

  //   PT_FMT(ypr[0], 2);
  //   PT('\t');
  //   PT_FMT(ypr[1], 2);
  //   PT('\t');
  //   PT_FMT(ypr[2], 2);
  // #ifdef PRINT_ACCELERATION
  //   PT("\t");
  //   // PT(aaWorld.x);
  //   // PT("\t");
  //   // PT(aaWorld.y);
  //   PT(*xyzReal[0]);  // x is along the longer direction of the robot
  //   PT("\t");
  //   PT(*xyzReal[1]);
  //   PT('\t');
  //   PT(*xyzReal[2]);
  //   PT("\t");
  //   PT(aaWorld.z);
  // #endif
  //   PTL();
}

TaskHandle_t TASK_imu = NULL;
// bool imuTaskRunning = true;

TaskHandle_t taskCalibrateImuUsingCore0_handle = NULL;  // -ee- Use to access taskCalibrateImuUsingCore0() running on Core 1 FROM Core 1

bool readIMU() {
  bool updated = false;
  if (updateGyroQ && !(frame % imuSkip)) {
#ifndef USE_WIRE1
    // Use FreeRTOS delay function and add timeout mechanism
    unsigned long waitStart = millis();
    const unsigned long maxWaitTime = 500; // Maximum wait time 500ms
    
    while (cameraLockI2c && updateGyroQ) {
      if (millis() - waitStart > maxWaitTime) {
        PTLF("Camera I2C lock timeout");
        return false; // Timeout exit
      }
      vTaskDelay(1 / portTICK_PERIOD_MS);  // Use FreeRTOS delay function
    }
#endif
    // Reuse existing timeout timer variable
    waitStart = millis(); // Reset timeout timer
    
    while (gestureLockI2c && updateGyroQ) {
      if (millis() - waitStart > maxWaitTime) {
        PTLF("Gesture I2C lock timeout");
        return false; // Timeout exit
      }
      vTaskDelay(1 / portTICK_PERIOD_MS);  // Use FreeRTOS delay function
    }
    
    waitStart = millis(); // Reset timer
    while (eepromLockI2c && updateGyroQ) {
      if (millis() - waitStart > maxWaitTime) {
        PTLF("EEPROM I2C lock timeout");
        return false; // Timeout exit
      }
      vTaskDelay(1 / portTICK_PERIOD_MS);  // Use FreeRTOS delay function
    }
    
    // If updateGyroQ became false during waiting, exit early
    if (!updateGyroQ) {
      return false;
    }
    imuLockI2c = true;
    
    // Final check before IMU operations - exit if requested
    if (!updateGyroQ) {
      imuLockI2c = false;  // Release lock before exiting
      return false;
    }
    
#ifdef IMU_ICM42670
    if (icmQ) {
      updated = true;
      icm.getImuGyro();
      for (byte i = 0; i < 3; i++) {
        // ICM42670's a_real is already in g units from transformIMUDataWithOffset()
        // Multiply by GRAVITY to match the expected scale (10.0 ≈ 1g)
        // Don't modify icm.a_real[i] directly to avoid cumulative multiplication
        xyzReal[i] = icm.a_real[i] * GRAVITY;
        ypr[i] = icm.ypr[i];
      }
      // Negate yaw to match polar coordinate convention (positive = counterclockwise)
      ypr[0] = -ypr[0];
    }
#endif
#ifdef IMU_MPU6050
    // if programming failed, don't try to do anything
    // read a packet from FIFO
    if (mpuQ) {
      updated |= mpu.read_mpu6050();  // mpu6050's frequency is lower than icm42670
      for (byte i = 0; i < 3; i++) {
        xyzReal[i] = mpu.a_real[i];
        ypr[i] = mpu.ypr[i];
      }
      // Negate yaw to match polar coordinate convention (positive = counterclockwise)
      ypr[0] = -ypr[0];
    }
#endif
    imuLockI2c = false;
    return updated;
  } else {
    vTaskDelay(1 / portTICK_PERIOD_MS);  // Use FreeRTOS delay function instead of delay()
    return false;
  }
}

// Wait for IMU readings to converge before evaluating exceptions
void waitForImuConvergence() {
  if (!updateGyroQ) return;
  
  const float YPR_THRESHOLD = 0.2;      // Degrees threshold for ypr convergence
  const float XYZ_THRESHOLD = 0.1;      // G-force threshold for acceleration convergence  
  const int MAX_ITERATIONS = 500;       // Maximum iterations to prevent infinite loop
  const int MIN_STABLE_READINGS = 5;    // Increased to 5 for better stability
  
  float prev_ypr[3] = {0, 0, 0};
  float prev_xyz[3] = {0, 0, 0};
  int stable_count = 0;
  int iteration = 0;
  
  PTL("Waiting for IMU convergence...");
  // Wait for taskIMU to start providing stable data
  // No need to lock I2C since we're not directly accessing IMU hardware
  PTL("Waiting for taskIMU to provide stable data...");
  
  // Initial reading from taskIMU processed data
  // Wait for taskIMU to update data at least once
  unsigned long waitStart = millis();
  while (!imuUpdated && (millis() - waitStart) < 1000) {
    delay(10);  // Wait for taskIMU to update
  }
  
  if (!imuUpdated) {
    PTL("Warning: taskIMU data not ready, using current values");
  }
  
  // Get initial values from taskIMU processed data
  for (int i = 0; i < 3; i++) {
    prev_ypr[i] = ypr[i];
    prev_xyz[i] = xyzReal[i];
  }
  
  while (iteration < MAX_ITERATIONS) {
    delay(20);  // Wait for taskIMU to update data
    
    // Wait for taskIMU to provide new data
    bool newDataAvailable = false;
    unsigned long dataWaitStart = millis();
    while (!newDataAvailable && (millis() - dataWaitStart) < 50) {
      if (imuUpdated) {  // taskIMU sets this flag when new data is available
        newDataAvailable = true;
      } else {
        delay(1);  // Wait for taskIMU to update
      }
    }
    
    // Check if we got new data from taskIMU
    if (newDataAvailable) {
      bool converged = true;
      // print6Axis();
      PT('.');
      
      // Calculate overall ypr difference using vector distance
      float ypr_diff = sqrt(pow(ypr[0] - prev_ypr[0], 2) + 
                           pow(ypr[1] - prev_ypr[1], 2) + 
                           pow(ypr[2] - prev_ypr[2], 2));
      
      // Calculate overall xyz difference using vector distance  
      float xyz_diff = sqrt(pow(xyzReal[0] - prev_xyz[0], 2) + 
                           pow(xyzReal[1] - prev_xyz[1], 2) + 
                           pow(xyzReal[2] - prev_xyz[2], 2));
      
      // Print convergence info right after IMU data, no newline
      // PTH("\t\t\t\t\t\tYPR diff: ", ypr_diff);
      // PTH(", XYZ diff: ", xyz_diff);
      // PTH(", Remaining: ", MAX_ITERATIONS - iteration);
      // PTL("");
      
      // Check convergence based on overall vector differences
      if (ypr_diff > YPR_THRESHOLD || xyz_diff > XYZ_THRESHOLD) {
        converged = false;
      }
      
      if (converged) {
        stable_count++;
        if (stable_count >= MIN_STABLE_READINGS) {
          PTL("\nIMU converged successfully");
          break;
        }
      } else {
        stable_count = 0;  // Reset stable count if not converged
        // Update previous values
        for (int i = 0; i < 3; i++) {
          prev_ypr[i] = ypr[i];
          prev_xyz[i] = xyzReal[i];
        }
      }
    }
    
    iteration++;
  }
  
  if (iteration >= MAX_ITERATIONS) {
    PTL("\nIMU convergence timeout - proceeding anyway");
  }
  
  PTL("\nIMU convergence detection completed using taskIMU data");
}

void getImuException() {
  // imuException = aaReal.z < 0 && fabs(ypr[2]) > 85;  //the second condition is used to filter out some noise

  // Acceleration Real
  //      ^ head
  //        ^ x+
  //        |
  //  y+ <------ y-
  //        |
  //        | x-
  // if (AWZ < -8500 && AWZ > -8600)
  //   imuException = IMU_EXCEPTION_FREEFALL;  //free falling
  // else
  // int8_t shockDirections = 0;
  // for (int8_t i = 0; i < 3; i++)
  //   if (fabs(xyzReal[i] - previousXYZ[i]) > 6000 * gFactor)
  //     shockDirections++;
  // PTHL("shockDirections", shockDirections);
  // PTT(fabs(xyzReal[0] - previousXYZ[0]), '\t');
  // PTT(fabs(xyzReal[1] - previousXYZ[1]), '\t');
  // PTT(fabs(xyzReal[2] - previousXYZ[2]), '\t');

  if (fabs(ypr[2]) > 85) {  //  imuException = aaReal.z < 0;
    if (mpuQ) {  // MPU is faster in detecting instant acceleration which may lead to false positive
      if (xyzReal[2] < 1)
        imuException = IMU_EXCEPTION_FLIPPED;  // flipped
    } else if (xyzReal[2] < -1)
      imuException = IMU_EXCEPTION_FLIPPED;  // flipped
  }
#ifndef ROBOT_ARM
  else if (ypr[1] < -50 || ypr[1] > 75)
    imuException = IMU_EXCEPTION_LIFTED;
  else if (!moduleDemoQ && fabs(xyzReal[2] - previousXYZ[2]) > thresZ * gFactor
           && fabs(xyzReal[2]) > thresZ * gFactor)  // Z direction shock
    imuException = IMU_EXCEPTION_KNOCKED;
  else if (!moduleDemoQ
           && (  // not in demo mode
               (fabs(xyzReal[0] - previousXYZ[0]) > 4000 * gFactor
                && fabs(xyzReal[0]) > thresX * gFactor)  // X direction shock
               || (fabs(xyzReal[1] - previousXYZ[1]) > 6000 * gFactor
                   && fabs(xyzReal[1]) > thresY * gFactor)  // Y direction shock
               )) {
    imuException = IMU_EXCEPTION_PUSHED;
  }
#endif
  // else if (  //keepDirectionQ &&
  //   fabs(previous_ypr[0] - ypr[0]) > 15 && fabs(fabs(ypr[0] - previous_ypr[0]) - 360) > 15)
  //   imuException = IMU_EXCEPTION_OFFDIRECTION;
  else if (turningQ) { // ** it's better to move this logic to taskQueue.h, make timing and turning parallel contidions
    // Check if we've reached the target turning angle
    // ypr[0] is already negated to match polar coordinate convention
    float currentYawDiff = ypr[0] - initialYawAngle;
    
    // Normalize the difference to -180 to 180 range
    while (currentYawDiff > 180) currentYawDiff -= 360;
    while (currentYawDiff < -180) currentYawDiff += 360;
    
    float targetDiff = targetYawAngle - initialYawAngle;
    while (targetDiff > 180) targetDiff -= 360;
    while (targetDiff < -180) targetDiff += 360;
    
    // Check if we've reached or exceeded the target angle
    // Special handling for ±180 degree boundary
    bool targetReached = false;
    if (fabs(targetDiff) >= 179.5) {
      // For ~180 degree turns, check if we're close to the opposite side
      targetReached = fabs(currentYawDiff - targetDiff) < 5.0 || fabs(fabs(currentYawDiff) - 180) < 5.0;
    } else if (targetDiff > 0) {
      targetReached = currentYawDiff >= targetDiff;
    } else if (targetDiff < 0) {
      targetReached = currentYawDiff <= targetDiff;
    }
    
    if (targetReached) {
      imuException = IMU_EXCEPTION_TURNING;
      turningQ = false;  // Stop turning control
      needTurning = true;  // Set flag to prevent exception from being skipped
      PTH("Turning target reached! Current yaw: ", ypr[0]);
      PTHL(" Target was: ", targetYawAngle);
    } else {
      imuException = 0;
    }
  } else if (!needTurning)  // Only reset exception if not waiting for turning processing
    imuException = 0;
  for (byte m = 0; m < 3; m++) {
    previousXYZ[m] = xyzReal[m];
    if (fabs(ypr[0] - previous_ypr[0]) < 2 || fabs(fabs(ypr[0] - previous_ypr[0]) - 360) < 2) {
      previous_ypr[m] = ypr[m];
    }
  }
}

long imuTime = 0;
void taskIMU(void *parameter) {
  bool* running = (bool*)parameter;
  // PTHL("updateGyroQ", updateGyroQ);
  // PTHL("run para:", *running);
  
  // unsigned long lastDebugPrint = 0;
  // const unsigned long debugInterval = 5000;  // print every 5 seconds
  
  while (*running) { // check pointer value and global variable
    // periodic print debug information
    // if (millis() - lastDebugPrint > debugInterval) {
      // PTHL("Task running, updateGyroQ =", updateGyroQ);
      // PTHL("*running =", *running);
      // lastDebugPrint = millis();
    // }
    if (millis() - imuTime > 5) {
      imuUpdated = readIMU();
      getImuException();
      imuTime = millis();
    } else
    {
      // ensure task is not blocked
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    // for checking the size of unused stack space
    // vTaskDelay(50 / portTICK_PERIOD_MS);  
    // Serial.println("Stack high water mark: " + String(uxTaskGetStackHighWaterMark(NULL)) + " bytes");
  }
  
  PTHL("before delete, updateGyroQ =", updateGyroQ);
  PTHL("*running =", *running);
  PTLF("IMU task exiting, calling vTaskDelete...");
  
  // ensure task can exit correctly
  vTaskDelay(10 / portTICK_PERIOD_MS);   // Reduce delay to ensure fast exit
  
  // Force delete task
  vTaskDelete(NULL);
}

void taskCalibrateImuUsingCore0(void *parameter);  // Forward declaration -ee-

void imuSetup() {
  if (newBoard) {
#ifndef AUTO_INIT
    PTL("- Calibrate the Inertial Measurement Unit (IMU)? (Y/n): ");
    char choice = getUserInputChar();
    PTL(choice);
    if (choice == 'Y' || choice == 'y')
      calibrateQ = true;
    if (calibrateQ) {
      PTLF("\nPut the robot FLAT on the table and don't touch it during calibration.");
      beep(8, 500, 500, 5);
    }
#else
    calibrateQ = true;
#endif
    beep(15, 500, 500, 1);
  }
#ifdef IMU_MPU6050
  if (mpuQ) {
    mpu.mpu6050Setup(calibrateQ);
  }
#endif
#ifdef IMU_ICM42670
  if (icmQ) {
    icm42670Setup(calibrateQ);
  }
#endif
  if (calibrateQ)
    beep(18, 50, 50, 6);
  previous_ypr[0] = ypr[0];
  
  // ensure updateGyroQ is true
  updateGyroQ = true;
  
  // Create IMU task
  xTaskCreatePinnedToCore(taskIMU,        // task function
                          "TaskIMU",      // task name
                          1500,           // task stack size​​
                          &updateGyroQ,   // parameters
                          1,              // priority
                          &TASK_imu,      // handle
                          0);             // core
  
  // wait for task creation to complete
  delay(100);
  
  // get task handle, for subsequent operations
  if (TASK_imu == NULL) {
    TASK_imu = xTaskGetHandle("TaskIMU");
    if (TASK_imu == NULL) {
      PTLF("Warning: Failed to get IMU task handle!");
    } else {
      PTLF("IMU task created successfully");
    }
  }

  // imuException = xyzReal[3] < 0;
}

void taskCalibrateImuUsingCore0(void *parameter) {
  /*  This perpetual task runs will run on Core 0 and will always wait for a notification from Core 1 before calibrating
     the IMU. Created by este este
  */
  // while (true) {
  // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // Wait for notification from Core 1
  //                                           // Received notification so do task work
  
#ifdef IMU_MPU6050
  if (mpuQ) {
    printToAllPorts("\n\t*** Current IMU Offsets ***");  // Show current offsets before doing the calibration
    mpu.PrintActiveOffsets();
    printToAllPorts("\n\t*** Calibrating Now ***\n");
    mpu.calibrateMPU();
  }
#endif
#ifdef IMU_ICM42670
  // IMU_ICM42670 lacks PrintActiveOffsets() method so need to do this piecemeal
  if (icmQ) {
    icm.getOffset(200);

    if (icm.offset_gyro[0] == -32768 || icm.offset_gyro[1] == -32768 || icm.offset_gyro[2] == -32768) {
      PT("Reading error: ");
      PT(icm.offset_gyro[0]);
      PT('\t');
      PT(icm.offset_gyro[1]);
      PT('\t');
      PT(icm.offset_gyro[2]);
      PTL('\t');
      while (!Serial.available()) {
        playMelody(imuBad2, sizeof(imuBad2) / 2);
      }
      while (Serial.available()) {
        Serial.read();
      }
    }

    PT("Current IMU Offsets:");  // Show current offsets before doing the calibration
    for (byte i = 0; i < 3; i++) {
      PTT(icm.offset_accel[i], '\t');
    }
    for (byte i = 0; i < 3; i++) {
      PTT(icm.offset_gyro[i], '\t');
    }
    PTL();

    printToAllPorts("\nCalibrating...\n");
    calibrateICM();
  }
#endif
  
  // Loop to resume waiting
  // }
  updateGyroQ = true;
  printToAllPorts("\nCalibration done.\n");  // for confirming the desktop app
  vTaskDelete(NULL);  // Terminate this task if an error occurs in the loop
}

#endif
