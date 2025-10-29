// code provided by the manufacturer
// modified by Rongzhong Li for better demonstration.
// Feb.16, 2021

#include "RgbUltrasonic.h"

#ifdef BiBoard_V1_0
RgbUltrasonic ultrasonic(UART_RX2, UART_TX2);  //(UART_RX, UART_TX);  //(signal, RGB) use the Grove Tx Rx
#else
RgbUltrasonic ultrasonic(UART_RX2, UART_TX2);  //(signal, RGB) use the Grove Tx Rx
// RgbUltrasonic ultrasonic(27, 23);  //(signal, RGB) use the infrared reciever's pin 23 and pwm pin 27
#endif
// The RGB LED ultrasonic module should be plugged in the fourth grove socket with D6, D7

int interval = 3;
#define ULTRASONIC_IMU_SKIP 10
long colors[] = { RGB_RED, RGB_PURPLE, RGB_GREEN, RGB_BLUE, RGB_YELLOW, RGB_WHITE };
long ultraTimer;
int ultraInterval = 1000;
int distance;

#ifdef NYBBLE
int feedbackDirection = 1;
#else
int feedbackDirection = -1;
#endif
bool ultrasonicLEDinitializedQ = false;
void logoColor() {
  ultrasonic.SetRgbEffect(E_RGB_RIGHT, ultrasonic.color(0, 28, 255), E_EFFECT_STEADY);  // the side is defined subjectively as the cat
  ultrasonic.SetRgbEffect(E_RGB_LEFT, ultrasonic.color(255, 171, 0), E_EFFECT_STEADY);
}
void rgbUltrasonicSetup() {
  ultrasonic.SetupLED();
  PTL("Show Petoi Logo color");
  logoColor();
  ultrasonicLEDinitializedQ = true;
}
int distanceRange = 0;
void read_RGBultrasonic() {
  if (millis() - ultraTimer > ultraInterval) {  //|| token == T_SKILL && millis() - ultraTimer > 3000) {
    ultraTimer = millis();
    // ultraInterval = 40;
    randomInterval = 1000;
    distance = ultrasonic.GetUltrasonicDistance();
    PTHL("distance", distance);
    if (distance > 60 && distanceRange != 0) {
      logoColor();
      distanceRange = 0;
      return;
    }
    if (distance > 50) {
      if (!manualEyeColorQ && distanceRange != 1) {
        ultrasonic.SetRgbEffect(E_RGB_ALL, RGB_WHITE, E_EFFECT_STEADY);
        distanceRange = 1;
      }
      ultraInterval = 3000;
      //      autoSwitch = true;
      randomInterval = 1000;
    } else if (distance > 40) {
      if (!manualEyeColorQ && distanceRange != 2) {
        ultrasonic.SetRgbEffect(E_RGB_ALL, RGB_YELLOW, E_EFFECT_STEADY);
        distanceRange = 2;
      }
    } else if (distance > 30) {
      if (!manualEyeColorQ && distanceRange != 3) {
        ultrasonic.SetRgbEffect(E_RGB_ALL, RGB_BLUE, E_EFFECT_STEADY);
        distanceRange = 3;
      }
    } else if (distance < 5) {
      if (!manualEyeColorQ && distanceRange != 4) {
        ultrasonic.SetRgbEffect(E_RGB_ALL, RGB_RED, E_EFFECT_STEADY);
        distanceRange = 4;
      }
      ultraInterval = 2000;
      randomInterval = 5000;
      if (skill->period > 1) {
        int dice = rand() % 4;
        if (dice == 0)
          tQueue->addTask('k', "bkL", 1000);
        else if (dice == 1)
          tQueue->addTask('k', "bkR", 1000);
        else if (dice == 2)
          tQueue->addTask('k', "snf", 1000);
        else if (dice == 3)
          tQueue->addTask('k', "sit", 1000);
      } else {
        tQueue->addTask('k', "sit", 1000);
      }
    } else if (distance < 10) {
      ultraInterval = 500;
      if (!manualEyeColorQ && distanceRange != 5) {
        ultrasonic.SetRgbEffect(E_RGB_ALL, RGB_PURPLE, E_EFFECT_STEADY);
        distanceRange = 5;
      }
      meow(rand() % 3 + 1, distance * 2);
      int amplitude = 5;
      int allRand[] = { 0, currentAng[0] + rand() % 20 - 10, 1, currentAng[1] + rand() % (2 * amplitude) - amplitude, 2, currentAng[2] + rand() % (amplitude * 4) - amplitude * 2 };
      int argLen = sizeof(allRand) / sizeof(int);
      for (byte i = 0; i < argLen; i++)
        newCmd[i] = (int8_t)allRand[i];
      printList(allRand, argLen);
      newCmd[argLen] = '~';
      tQueue->addTask('I', newCmd, 50);
      tQueue->addTask('i', "");
    } else if (distance < 15) {
      if (!manualEyeColorQ && distanceRange != 6) {
        ultrasonic.SetRgbColor(E_RGB_ALL, RGB_GREEN);
        tQueue->addTask('k', "sit", 2000);
        // tQueue->addTask('k', "up");
        ultraInterval = 1000;
        distanceRange = 6;
      }
    } else {  // 15~30
      ultraInterval = 1000;
      distance -= 15;
      if (!manualEyeColorQ && distanceRange != 7) {
        ultrasonic.SetRgbColor(E_RGB_ALL, colors[max(min(distance / 3, 5), 0)]);
        distanceRange = 7;
      }
      if (skill->period > 1) {
        int dice = rand() % 3;
        if (dice == 0)
          tQueue->addTask('k', "wkL", 1000);
        else if (dice == 1)
          tQueue->addTask('k', "wkR", 1000);
        else if (dice == 2)
          tQueue->addTask('q', "ksit:100>i0 20 1 0 8 -70 12 0 15 10:0>o1 0, 0 40 -20 4 0, 1 -30 20 4 30, 8 -70 10 4 60, 12 -10 10 4 0, 15 10 0 4 0:100>m0 0 1 -20 2 0:0>ksit:1000");
        if (dice < 4)
          tQueue->addTask('k', "wkF", 2000);
      }
      // #ifdef NYBBLE
      //       int mid[] = {
      //         15, 0, 0, 0,       //
      //         0, 0, 0, 0,        //
      //         30, 30, -30, -30,  //
      //         30, 30, -30, -30,  //
      //       };
      // #else
      //       int mid[] = {
      //         15, 0, 0, 0,       //
      //         0, 0, 0, 0,        //
      //         30, 30, 90, 90,    //
      //         30, 30, -30, -30,  //
      //       };
      // #endif
      //       int allParameter[] = { mid[0] - distance / 4, -10 + distance / 4, distance * (random() % 50 < 1 ? int(random() % 2 - 1) : 1)/2, 0,
      //                              0, 0, 0, 0,
      //                              mid[8] - 15 + distance / 4, mid[9] - 15 + distance / 4, mid[10] - 30 + distance * feedbackDirection/2, mid[11] - 30 + distance * feedbackDirection/2,
      //                              mid[12] + 35 - distance/2, mid[13] + 35 - distance/2, mid[14] + 40 - distance * feedbackDirection/2, mid[15] + 40 - distance * feedbackDirection/2 };
      //       //      printList(allParameter);
      //       cmdLen = 16;
      //       for (byte i = 0; i < cmdLen; i++)
      //         newCmd[i] = (int8_t)min(max(allParameter[i], -128), 127);
      //       newCmd[cmdLen] = '~';
      //       randomInterval = 5000;
      //       if (tQueue->cleared())
      //         tQueue->addTask('L', newCmd);
    }
  }
}

float readUltrasonic(int trigger, int echo = -1) {  // give two parameters for the traditional ultrasonic sensor
  // give one parameter for the one pin ultrasonic sensor that shares the trigger and echo pins
  if (echo == -1)
    echo = trigger;
  int longestDistance = 200;  // 200 cm = 2 meters. the sensor can only measure up to 200 cm or less. 
  float farTime = longestDistance * 2 / 0.034;
  pinMode(trigger, OUTPUT);

  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  pinMode(echo, INPUT);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(echo, HIGH, farTime);
  // Calculating the distance
  if (duration == 0)
    return -1;
  else
    return duration * 0.034 / 2;  // 10^-6 * 34000 cm/s
}
