#include "esp32-hal.h"
/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE"
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "bleCommon.h"

#define HOLD_TIME 1500        // Increase to 3 seconds (3000), improve connection stability  
// #define CONNECTION_ATTEMPT 3  // Increase reconnection attempts
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// BLE UUID definitions are now in bleCommon.h

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};
byte bleMessageShift = 1;
int buffLen = 0;
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // Use C-style string handling to avoid std::string compatibility issues
    const char *rxValueCStr = pCharacteristic->getValue().c_str();
    buffLen = pCharacteristic->getValue().length();
    // String rxValue = String(pCharacteristic->getValue().c_str());//it will cause unkown bug when sending 'K'-token skill data via mobile app and break the main program.
    if (buffLen > 0) {
      // long current = millis();
      // PTH("BLE", current - lastSerialTime);
      // Serial.print("From BLE:");
      // Serial.println(rxValueCStr);
      if (bleMessageShift) {
        cmdLen = 0;
        token = rxValueCStr[0];
        lowerToken = tolower(token);
        terminator = (token >= 'A' && token <= 'Z') ? '~' : '\n';
        serialTimeout = (token == T_SKILL_DATA || lowerToken == T_BEEP || token == T_TASK_QUEUE) ? SERIAL_TIMEOUT_LONG : SERIAL_TIMEOUT;
      }
      for (int i = bleMessageShift; i < buffLen; i++) {
        newCmd[cmdLen++] = rxValueCStr[i];
      }
      bleMessageShift = 0;
      lastSerialTime = millis();
    }
    buffLen = 0;
  }
};

void readBle() {
  if (deviceConnected && !bleMessageShift) {
    while ((char)newCmd[cmdLen - 1] != terminator && long(millis() - lastSerialTime) < serialTimeout) {  // wait until the long message is completed
      delay(SERIAL_TIMEOUT);
    }
    // PTH("* BLE", millis() - lastSerialTime);
    cmdLen = (newCmd[cmdLen - 1] == terminator) ? cmdLen - 1 : cmdLen;
    newCmd[cmdLen] = (token >= 'A' && token <= 'Z') ? '~' : '\0';
    newCmd[cmdLen + 1] = '\0';
    newCmdIdx = 2;
    bleMessageShift = 1;
    if (token == 'g' && cmdLen == 0) {  // adapt for the mobile app where 'g' toggles acceleration
      cmdLen = 1;
      newCmd[0] = fineAdjustQ ? 'f' : 'F';
      newCmd[1] = '\0';
    }
  }
}

void bleSetup() {
  //  Serial.print("UUID: ");
  //  Serial.println(SERVICE_UUID_APP);
  // Create the BLE Device
  char *bleName = getDeviceName("_BLE");
  PTHL("BLE:\t", bleName);
  BLEDevice::init(bleName);  // read BLE device name from global uniqueName
  delete[] bleName;          // Free the allocated memory
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(BLE_SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic =
    pService->createCharacteristic(
      BLE_CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic =
    pService->createCharacteristic(
      BLE_CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a BLE client connection to notify...");
}

void bleWrite(String buff) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(buff.c_str());
    pTxCharacteristic->notify();
  }
}

// Add connection state debounce variables
unsigned long lastConnectionChange = 0;
const unsigned long CONNECTION_DEBOUNCE = 1000; // 1 second debounce

void detectBle() {
  unsigned long currentTime = millis();
  
  // Debounce processing: avoid frequent connection state changes
  if (currentTime - lastConnectionChange < CONNECTION_DEBOUNCE) {
    return;
  }
  
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(50);                    // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("Bluetooth BLE disconnected!");
    oldDeviceConnected = deviceConnected;
    lastConnectionChange = currentTime;  // Record state change time
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    Serial.println("Bluetooth BLE connected!");
    oldDeviceConnected = deviceConnected;
    delay(HOLD_TIME);
    
    // Send connection confirmation message

    // for (byte i = 0; i < CONNECTION_ATTEMPT; i++) {
    pTxCharacteristic->setValue((String("Petoi ") + MODEL).c_str());
    pTxCharacteristic->notify();
    //   delay(100);
    // }
    
    token = 'd';
    bleWrite(String(T_PAUSE));
    lastConnectionChange = currentTime;  // Record state change time
  }
}
