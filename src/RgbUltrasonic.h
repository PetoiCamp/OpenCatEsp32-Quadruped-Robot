#ifndef _RGBULTRASONIC_H_
#define _RGBULTRASONIC_H_

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
// #include "RGBLed.h"
#include "Adafruit_NeoPixel/Adafruit_NeoPixel.h"
#define UL_LIMIT_MIN 5
#define UL_LIMIT_MID 10
#define UL_LIMIT_MAX 400

// #define RGB_RED 0x220000
// #define RGB_GREEN 0x002200
// #define RGB_BLUE 0x000022
// #define RGB_YELLOW 0x222200
// #define RGB_PURPLE 0x220022
// #define RGB_WHITE 0x222222

#define RGB_RED     0xFF0000
#define RGB_GREEN   0x00FF00
#define RGB_BLUE    0x0000FF
#define RGB_YELLOW  0xFFFF00
#define RGB_PURPLE  0xFF00FF
#define RGB_WHITE   0xFFFFFF

typedef enum {
  E_RGB_ALL = 0,
  E_RGB_RIGHT = 1,
  E_RGB_LEFT = 2
} E_RGB_INDEX;

typedef enum {
  E_EFFECT_BREATHING = 0,
  E_EFFECT_ROTATE = 1,
  E_EFFECT_FLASH = 2,
  E_EFFECT_STEADY = 3,
} E_RGB_EFFECT;

class RgbUltrasonic {
private:
  byte SignalPin, RgbPin;
  Adafruit_NeoPixel *mRgb;

public:
  float FrontDistance;
  int measurementInterval;
  long lastMeasurementTime;
  long color(uint8_t red, uint8_t green, uint8_t blue) {
    return ((long)(red) << 16) + ((long)(green) << 8) + (long)(blue);
  }
  RgbUltrasonic(byte signal_pin, byte rgb_pin) {
    SignalPin = signal_pin;
    RgbPin = rgb_pin;
    lastMeasurementTime = millis();
    measurementInterval = 20;
    Serial.println("set up ultrasonic sensor");
    delay(100);
  }
  float GetUltrasonicDistance(void) {  // in cm
    if (millis() - lastMeasurementTime >= measurementInterval) {
      unsigned long Time_Echo_us = 0;
      pinMode(SignalPin, OUTPUT);
      digitalWrite(SignalPin, LOW);
      delayMicroseconds(2);
      digitalWrite(SignalPin, HIGH);
      delayMicroseconds(20);
      digitalWrite(SignalPin, LOW);
      pinMode(SignalPin, INPUT);
      Time_Echo_us = pulseIn(SignalPin, HIGH);
      if ((Time_Echo_us < measurementInterval * 1000) && (Time_Echo_us > 1)) {  // max 172cm
        FrontDistance = Time_Echo_us / 58.00;
      }
      lastMeasurementTime = millis();
    }
    return FrontDistance;
  }
  void SetupLED() {
    mRgb = new Adafruit_NeoPixel(6, RgbPin, NEO_GRB + NEO_KHZ800);
    mRgb->begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
    mRgb->show();             // Turn OFF all pixels ASAP
    mRgb->setBrightness(50);  // Set BRIGHTNESS to about 1/5 (max = 255)
    delay(100);
  }
  void SetRgbColor(E_RGB_INDEX index, long Color) {
    if (index == E_RGB_ALL)
      for (byte i = 0; i < 6; i++) {
        mRgb->setPixelColor(i, 0);
        mRgb->show();  // the light has to be refreshed
        mRgb->setPixelColor(i, Color);
      }
    else if (index == E_RGB_RIGHT) {
      for (byte i = 0; i < 3; i++) {
        mRgb->setPixelColor(i, 0);
        mRgb->show();
        mRgb->setPixelColor(i, Color);
      }

    } else if (index == E_RGB_LEFT) {
      for (byte i = 3; i < 6; i++) {
        mRgb->setPixelColor(i, 0);
        mRgb->show();
        mRgb->setPixelColor(i, Color);
      }
    }
    mRgb->show();
  }
  void SetRgbEffect(E_RGB_INDEX index, long Color, uint8_t effect) {
    switch ((E_RGB_EFFECT)effect) {
      case E_EFFECT_BREATHING:
        long rgb[3];
        for (byte c = 0; c < 3; c++) {
          rgb[c] = Color >> (2 - c) * 8 & 0x0000FF;
        }
        for (byte i = 5; i < 255; i += 5) {
          SetRgbColor(index, color(max(rgb[0] - i, long(5)), max(rgb[1] - i, long(5)), max(rgb[2] - i, long(5))));
          delay((i < 20) ? 10 : (256 / i));
        }
        for (byte i = 255; i >= 5; i -= 5) {  // avoid the light completely turning off
          SetRgbColor(index, color(max(rgb[0] - i, long(5)), max(rgb[1] - i, long(5)), max(rgb[2] - i, long(5))));
          delay((i < 20) ? 10 : (256 / i));
        }
        break;
      case E_EFFECT_ROTATE:
        SetRgbColor(E_RGB_ALL, 0);
        mRgb->setPixelColor(0, Color);
        mRgb->setPixelColor(3, Color);
        mRgb->show();
        delay(100);
        mRgb->setPixelColor(0, 0);
        mRgb->setPixelColor(3, 0);
        mRgb->setPixelColor(1, Color);
        mRgb->setPixelColor(4, Color);
        mRgb->show();
        delay(100);
        mRgb->setPixelColor(1, 0);
        mRgb->setPixelColor(4, 0);
        mRgb->setPixelColor(2, Color);
        mRgb->setPixelColor(5, Color);
        mRgb->show();
        delay(100);
        mRgb->setPixelColor(2, 0);
        mRgb->setPixelColor(5, 0);
        mRgb->show();
        delay(100);
        SetRgbColor(E_RGB_ALL, Color);
        break;
      case E_EFFECT_FLASH:
        for (byte i = 0; i < 3; i++) {
          SetRgbColor(E_RGB_ALL, 0);
          delay(100);
          SetRgbColor(E_RGB_ALL, Color);
          delay(100);
        }
        break;
      default:
        SetRgbColor(index, Color);
        break;
    }
  }
};
#endif
