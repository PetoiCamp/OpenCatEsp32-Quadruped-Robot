/*
  APDS-9960 - Gesture Sensor
  https://www.arduino.cc/reference/en/libraries/arduino_apds9960/

  This example reads gesture data from the on-board APDS-9960 sensor of the
  Nano 33 BLE Sense and prints any detected gestures to the Serial Monitor.

  Gesture directions are as follows:
  - UP:    from USB connector towards antenna
  - DOWN:  from antenna towards USB connector
  - LEFT:  from analog pins side towards digital pins side
  - RIGHT: from digital pins side towards analog pins side

  The circuit:
  - Arduino Nano 33 BLE Sense

  This example code is in the public domain.
*/

#include "gesture9960/Arduino_APDS9960.h"

bool gestureReactionQ = true;
int8_t gesturePrintQ = 0;
int gestureGetValue = -1;

void gestureSetup() {
  if (!APDS.begin()) {
    PTLF("Error initializing APDS-9960 sensor!");
  }
  // for setGestureSensitivity(..) a value between 1 and 100 is required.
  // Higher values make the gesture recognition more sensitive but less accurate
  // (a wrong gesture may be detected). Lower values makes the gesture recognition
  // more accurate but less sensitive (some gestures may be missed).
  // Default is 80
  //APDS.setGestureSensitivity(80);

  PTLF("Detecting gestures ...");
}

void gestureStop() {
  PTLF("Stopping gesture detection ...");
  APDS.end();
}

int8_t melody54321[] = { 19, 64, 17, 64, 16, 64, 14, 64, 12, 32, '~' };
int8_t melody12345[] = { 12, 64, 14, 64, 16, 64, 17, 64, 19, 32, '~' };
int8_t melody67345[] = { 21, 16, 23, 32, 16, 64, 17, 64, 19, 64, '~' };
int8_t melody32654[] = { 16, 64, 14, 16, 21, 64, 19, 32, 17, 16, '~' };

unsigned long lastValidGestureTime = 0;
const unsigned long GESTURE_MIN_INTERVAL = 1500;  // Minimum 1.5 seconds between gestures

int read_gesture() {
  // if(millis() - lastValidGestureTime > 5000){
#ifndef USE_WIRE1
  while (cameraLockI2c)
    delay(1);  //wait for the camera to release lock. potentially to cause dead lock with camera
#endif
#ifdef GYRO_PIN
  while (imuLockI2c)
    delay(1);  //wait for the imu to release lock. potentially to cause dead lock with camera
#endif
  while (eepromLockI2c)
    delay(1);  //wait for the EEPROM operations to complete
  gestureLockI2c = true;
  int gesture = GESTURE_NONE;
  if (APDS.gestureAvailable()) 
  {
    // a gesture was detected, read and print to Serial Monitor
    gesture = APDS.readGesture();
    // Check task queue size to prevent overflow (max 6 tasks)
    if (gestureReactionQ) 
    {
      // Check minimum time interval between gestures
      unsigned long currentTime = millis();
      if (currentTime - lastValidGestureTime < GESTURE_MIN_INTERVAL) {
        PTLF("Gesture too soon, skipping");
        gestureLockI2c = false;
        return gesture;
      }
      
      if (tQueue->size() >= 6) {
        PTLF("Task queue full, skipping gesture");
        gestureLockI2c = false;
        return gesture;
      }
      
      lastValidGestureTime = currentTime;
      PTF("Detected ");
      switch (gesture) 
      {
        case GESTURE_UP:
          {
            PTLF("UP\t↑");
            tQueue->addTask(T_INDEXED_SEQUENTIAL_ASC, "1 90 1 0", 0);
            tQueue->addTask(T_BEEP_BIN, melody12345, 0);
            tQueue->addTask('k', "fiv", 2000);
            break;
          }

        case GESTURE_DOWN:
          {
            PTLF("DOWN\t↓");
            tQueue->addTask('i', "");
            tQueue->addTask(T_BEEP_BIN, melody54321, 0);
            tQueue->addTask('k', "sit");
            tQueue->addTask('k',
#ifdef NYBBLE
                            "wsf"
#else
                            "scrh"
#endif
            );
            break;
          }

        case GESTURE_LEFT:
          {
            PTLF("LEFT\t←");
            int8_t move[] = { 0, -60, 0, -55, '~' };
            tQueue->addTask(T_BEEP_BIN, melody32654, 0);
            tQueue->addTask(T_INDEXED_SEQUENTIAL_BIN, move, 1000);
            tQueue->addTask(T_INDEXED_SIMULTANEOUS_ASC, "0 0");
            break;
          }
        case GESTURE_RIGHT:
          {
            PTLF("RIGHT\t→");
            int8_t move[] = { 0, 60, 0, 55, '~' };
            tQueue->addTask(T_BEEP_BIN, melody67345, 0);
            tQueue->addTask(T_INDEXED_SEQUENTIAL_BIN, move, 0);
            tQueue->addTask(T_INDEXED_SIMULTANEOUS_ASC, "0 0");
            break;
          }
        default:
          // ignore
          break;
      }
      // lastValidGestureTime = millis();
    }
  }
  gestureLockI2c = false;
  // PTHL("gestureValue01:", gesture);
  return gesture;
  // }
}
