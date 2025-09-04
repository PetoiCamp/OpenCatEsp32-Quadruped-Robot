#ifdef BT_BLE
#include "bleUart.h"
#endif
#ifdef BT_CLIENT
#include "bleClient.h"
#endif

#ifdef BT_SSP
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#endif

// Bluetooth mode management variables and functions
#define BLUETOOTH_MODE_DEFINED
enum BluetoothMode {
  BT_MODE_NONE = 0,
  BT_MODE_SERVER = 1,
  BT_MODE_CLIENT = 2,
  BT_MODE_BOTH = 3
};

// BluetoothMode currentBtMode = BT_MODE_NONE;
BluetoothMode activeBtMode = BT_MODE_NONE;
unsigned long btModeDecisionStartTime = 0;  // 30 second decision timer
unsigned long btModeLastCheckTime = 0;      // 1 second check interval timer
const unsigned long BT_MODE_CHECK_INTERVAL = 1000; // Check every second
const unsigned long BT_MODE_DECISION_TIMEOUT = 30000; // 30 second decision timeout

void initBluetoothModes();
void checkAndSwitchBluetoothMode();
void shutdownBleServer();
void shutdownBleClient();

#ifdef BT_SSP
BluetoothSerial SerialBT;
boolean confirmRequestPending = true;
boolean BTconnected = false;

void BTConfirmRequestCallback(uint32_t numVal) {
  confirmRequestPending = true;
  Serial.print("SSP PIN: ");
  Serial.println(numVal);
  Serial.println("Auto-confirming SSP pairing...");
  SerialBT.confirmReply(true);    // Auto-confirm pairing request
  confirmRequestPending = false;
}

void BTAuthCompleteCallback(boolean success) {
  confirmRequestPending = false;
  if (success) {
    BTconnected = true;
    Serial.println("SSP Pairing success!!");
  } else {
    BTconnected = false;
    Serial.println("SSP Pairing failed, rejected by user!!");
  }
}

void blueSspSetup() {
  SerialBT.enableSSP();
  SerialBT.onConfirmRequest(BTConfirmRequestCallback);
  SerialBT.onAuthComplete(BTAuthCompleteCallback);
  char *sspName = getDeviceName("_SSP");
  PTHL("SSP:\t", sspName);
  SerialBT.begin(sspName);  // Bluetooth device name
  delete[] sspName;         // Free the allocated memory
  Serial.println("The SSP device is started, now you can pair it with Bluetooth!");
}

// void readBlueSSP() {
//   if (confirmRequestPending)
//   {
//     if (Serial.available())
//     {
//       int dat = Serial.read();
//       if (dat == 'Y' || dat == 'y')
//       {
//         SerialBT.confirmReply(true);
//       }
//       else
//       {
//         SerialBT.confirmReply(false);
//       }
//     }
//   }
//   else
//   {
//     if (Serial.available())
//     {
//       SerialBT.write(Serial.read());
//     }
//     if (SerialBT.available())
//     {
//       Serial.write(SerialBT.read());
//     }
//     delay(20);
//   }
// }

// end of Richard Li's code
#endif

// 蓝牙模式管理函数实现
void initBluetoothModes() {
  PTLF("Initializing Bluetooth modes...");
  
#if defined(BT_BLE) && defined(BT_CLIENT)
  // If both modes are defined, enable intelligent switching
  // currentBtMode = BT_MODE_BOTH;
  PTLF("Starting BLE Server...");
  bleSetup();
  delay(100);  // Give BLE server enough startup time
  
  PTLF("Starting BLE Client...");
  bleClientSetup();
  delay(100);  // Give BLE client enough startup time
  
  btModeDecisionStartTime = millis();  // Start 30 second decision timer
  btModeLastCheckTime = millis();      // Initialize check interval timer
  PTLF("Both BT modes started. Waiting for connection...");
  
#elif defined(BT_BLE)
  // Only start BLE server
  // currentBtMode = BT_MODE_SERVER;
  activeBtMode = BT_MODE_SERVER;
  bleSetup();
  PTLF("BLE Server mode activated");
  
#elif defined(BT_CLIENT)
  // Only start BLE client
  // currentBtMode = BT_MODE_CLIENT;
  activeBtMode = BT_MODE_CLIENT;
  bleClientSetup();
  PTLF("BLE Client mode activated");
  
#else
  // currentBtMode = BT_MODE_NONE;
  PTLF("No Bluetooth mode enabled");
#endif

#ifdef BT_SSP
  blueSspSetup();
#endif
}

void checkAndSwitchBluetoothMode() {
  if (activeBtMode != BT_MODE_NONE) {    // currentBtMode != BT_MODE_BOTH  
    return; // No need to switch or already switched
  }
  
  unsigned long currentTime = millis();
  
  // Check 30 second decision timeout (using independent timer)
  if (currentTime - btModeDecisionStartTime > BT_MODE_DECISION_TIMEOUT) {
    // After timeout, default to closing BLE client, keeping server mode
    PTLF("BT mode decision timeout, defaulting to Server mode");
    shutdownBleClient();
    activeBtMode = BT_MODE_SERVER;
    return;
  }
  
  // Check 1 second check interval
  if (currentTime - btModeLastCheckTime < BT_MODE_CHECK_INTERVAL) {
    return;
  }
  btModeLastCheckTime = currentTime;  // Only update check interval timer

#ifdef BT_CLIENT
  // Check if BLE client has connection
  if (btConnected) {
    PTLF("BLE Client connected, shutting down Server mode");
    shutdownBleServer();
    activeBtMode = BT_MODE_CLIENT;
    return;
  }
#endif
  
#if defined BT_BLE
  // Check if BLE server has connection to mobile app
  if (deviceConnected) {
    PTLF("BLE Server connected, shutting down Client mode");
    shutdownBleClient();
    activeBtMode = BT_MODE_SERVER;
    return;
  }
#endif
}

void shutdownBleServer() {
#ifdef BT_BLE
  if (pServer) {
    pServer->getAdvertising()->stop();
    PTLF("BLE Server advertising stopped");
  }
  deviceConnected = false;
#endif
}

void shutdownBleClient() {
#ifdef BT_CLIENT
  btConnected = false;
  doConnect = false;
  doScan = false;
  PTLF("BLE Client scanning stopped");
#endif
}