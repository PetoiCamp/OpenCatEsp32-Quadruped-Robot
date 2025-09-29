#ifdef BT_BLE
#include "bleUart.h"
#endif
#ifdef BT_CLIENT
#include "bleClient.h"
#endif

// 添加WiFi头文件以支持WiFi状态检查
#ifdef WEB_SERVER
#include <WiFi.h>
#endif

// 包含tools.h以获取调试打印宏的定义
#include "tools.h"

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

BluetoothMode activeBtMode = BT_MODE_NONE;
unsigned long btModeDecisionStartTime = 0;  // 3 second decision timer
unsigned long btModeLastCheckTime = 0;      // 1 second check interval timer
const unsigned long BT_MODE_CHECK_INTERVAL = 1000; // Check every second
const unsigned long BT_MODE_DECISION_TIMEOUT = 3000; // 3 second decision timeout

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
  
  // 如果WiFi正在连接，等待一下避免资源竞争
#ifdef WEB_SERVER
  if (WiFi.status() == WL_DISCONNECTED) {
    PTLF("Waiting for WiFi connection to stabilize before starting Bluetooth...");
    delay(1000);
  }
#endif
  
  btModeDecisionStartTime = millis();  // Start 3 second decision timer
  btModeLastCheckTime = millis();      // Initialize check interval timer
  // PTLF("Both BT modes started. Waiting for connection...");
  
#ifdef BT_CLIENT
  PTLF("Starting BLE Client...");
  bleClientSetup();
  delay(200);  // 增加启动时间，避免与WiFi冲突

  unsigned long currentTime = millis();
  
  // Check 3 second decision timeout (using independent timer)
  while (currentTime - btModeDecisionStartTime < BT_MODE_DECISION_TIMEOUT) {
    // 减少扫描频率，避免过度干扰WebSocket连接
    if (currentTime - btModeLastCheckTime >= BT_MODE_CHECK_INTERVAL) {
      checkBtScan();
      if (btConnected)
      {
        PTLF("BLE Client connected, shutting down Server mode");
        activeBtMode = BT_MODE_CLIENT;
      }
      btModeLastCheckTime = currentTime;
    }
    // 检查WebSocket连接状态，如果有活跃连接则减少扫描频率
#ifdef WEB_SERVER
    extern bool webServerConnected;
    extern std::map<uint8_t, bool> connectedClients;
    
    if (webServerConnected && !connectedClients.empty()) {
      delay(500); // 给WebSocket更多时间处理
    } 
#endif
    delay(100);
    currentTime = millis();
  }
  // After timeout, shut down client mode and start server mode
  if(activeBtMode != BT_MODE_CLIENT){
  PTLF("Shutting down BLE Client...");
  shutdownBleClient();
  delay(500); // 给BLE堆栈更多时间完成清理
  
  // 完全重新初始化BLE设备以切换到Server模式
  PTLF("Deinitializing BLE device...");
  BLEDevice::deinit(false); // 去初始化BLE设备，但保留内存
  delay(500); // 等待去初始化完成
  }
  if(activeBtMode != BT_MODE_CLIENT)
#endif
#if defined(BT_BLE)
  // Only start BLE server
  {
    activeBtMode = BT_MODE_SERVER;
  bleSetup();
  PTLF("BLE Server mode activated");
  }
#endif

#ifdef BT_SSP
  blueSspSetup();
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
  PetoiBtStopScan();
#endif
}
