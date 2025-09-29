//Generated Date: Fri, 20 Sep 2024 04:06:24 GMT by Jason
#include <BLEDevice.h>
#include "bleCommon.h"

#define DEVICE_NAME "BBC"

static BLEUUID serviceUUID(BLE_SERVICE_UUID);
static BLEUUID CHARACTERISTIC_UUID_RX(BLE_CHARACTERISTIC_UUID_RX);
static BLEUUID CHARACTERISTIC_UUID_TX(BLE_CHARACTERISTIC_UUID_TX);
static boolean doConnect = false;
static boolean btConnected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristicTx;
static BLERemoteCharacteristic* pRemoteCharacteristicRx;
static BLERemoteCharacteristic* pRemoteCharacteristicTemp;
static BLEAdvertisedDevice* PetoiBtDevice;
BLERemoteDescriptor* pRD;
bool btReceiveDone = false;
String btRxLoad = "";
uint8_t dataIndicate[2] = { 0x02, 0x00 };
// String serverBtDeviceName = "";

void bleParser(String raw) {
  String cmd = "";
  token = raw[0];
  strcpy(newCmd, raw.c_str() + 1);
  newCmdIdx = 2;
  cmdLen = strlen(newCmd);
}

void PetoiBtConnected() {
}

void PetoiBtDisconnected() {
}

static void btPetoiNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  btReceiveDone = false;
  if (length > 0) {
    btRxLoad = "";
    for (int i = 0; i < length; i++)
      btRxLoad += (char)pData[i];
  }
  btRxLoad.replace("\r", "");
  btRxLoad.replace("\n", "");
  /* For the ESP32 library v2.0.12 the following codes should be commented
//  if (pBLERemoteCharacteristic->canIndicate())
//  {
//    pRD->writeValue(dataIndicate, 2, false);
//  }
*/
  btReceiveDone = true;
}

// Add client connection state debounce variables
unsigned long lastClientConnectionChange = 0;
const unsigned long CLIENT_CONNECTION_DEBOUNCE = 1000; // 1 second debounce

class btPetoiClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    unsigned long currentTime = millis();
    
    // Debounce processing
    if (currentTime - lastClientConnectionChange < CLIENT_CONNECTION_DEBOUNCE) {
      return;
    }
    
    PetoiBtConnected();
    lastClientConnectionChange = currentTime;
  }

  void onDisconnect(BLEClient* pclient) {
    unsigned long currentTime = millis();
    
    // Debounce processing
    if (currentTime - lastClientConnectionChange < CLIENT_CONNECTION_DEBOUNCE) {
      return;
    }
    
    btConnected = false;
    btReceiveDone = false;
    btRxLoad = "";
    PetoiBtDisconnected();
    lastClientConnectionChange = currentTime;
  }
};

// Declare static variables for classes already defined
static btPetoiClientCallback* pClientCallback = nullptr;
static BLEClient* pBleClient = nullptr;

bool connectToServer() {
  // Clean up any existing client resources
  if (pBleClient != nullptr) {
    pBleClient->disconnect();
    delete pBleClient;
    pBleClient = nullptr;
  }
  if (pClientCallback != nullptr) {
    delete pClientCallback;
    pClientCallback = nullptr;
  }
  
  pBleClient = BLEDevice::createClient();
  pClientCallback = new btPetoiClientCallback();
  pBleClient->setClientCallbacks(pClientCallback);
  pBleClient->connect(PetoiBtDevice);
  BLERemoteService* pRemoteService = pBleClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    pBleClient->disconnect();
    return false;
  }
  pRemoteCharacteristicTx = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_TX);
  pRemoteCharacteristicRx = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_RX);
  if (pRemoteCharacteristicTx == nullptr) {
    pBleClient->disconnect();
    return false;
  }
  if (pRemoteCharacteristicRx == nullptr) {
    pBleClient->disconnect();
    return false;
  }
  if (!pRemoteCharacteristicTx->canWrite()) {
    pRemoteCharacteristicTemp = pRemoteCharacteristicTx;
    pRemoteCharacteristicTx = pRemoteCharacteristicRx;
    pRemoteCharacteristicRx = pRemoteCharacteristicTemp;
  }
  if (pRemoteCharacteristicRx->canIndicate() || pRemoteCharacteristicRx->canNotify()) {
    pRemoteCharacteristicRx->registerForNotify(btPetoiNotifyCallback);
    PTLF("===Registed===");
  }
  if (pRemoteCharacteristicRx->canIndicate()) {
    pRD = pRemoteCharacteristicRx->getDescriptor(BLEUUID((uint16_t)0x2902));
    if (pRD == nullptr) {
      pBleClient->disconnect();
      return false;
    }
    pRD->writeValue(dataIndicate, 2, false);
  }
  btConnected = true;
  return true;
}

class PetoiAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String tempDeviceName = advertisedDevice.getName().c_str();
    //  if (tempDeviceName.equals(serverBtDeviceName)) {
    if (strstr(advertisedDevice.getName().c_str(), DEVICE_NAME) != NULL) {
      BLEDevice::getScan()->stop();
      PetoiBtDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
      PTHL("Advertised Device found:", tempDeviceName);
    }
  }
};

// Declare static variable for PetoiAdvertisedDeviceCallbacks class
static PetoiAdvertisedDeviceCallbacks* pAdvertisedDeviceCallbacks = nullptr;

void PetoiBtStartScan() {
  // 检查WebSocket连接状态，如果有活跃连接则延迟扫描
#ifdef WEB_SERVER
  extern bool webServerConnected;
  extern std::map<uint8_t, bool> connectedClients;
  
  if (webServerConnected && !connectedClients.empty()) {
    PTLF("WebSocket clients active, delaying BLE scan...");
    delay(2000); // 等待WebSocket连接稳定
  }
#endif
  
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  
  // Clean up any existing callback instance before creating a new one
  if (pAdvertisedDeviceCallbacks != nullptr) {
    delete pAdvertisedDeviceCallbacks;
    pAdvertisedDeviceCallbacks = nullptr;
  }
  
  // Create and save the callback instance
  pAdvertisedDeviceCallbacks = new PetoiAdvertisedDeviceCallbacks();
  pBLEScan->setAdvertisedDeviceCallbacks(pAdvertisedDeviceCallbacks);
  
  // Optimize scan parameters to minimize WiFi interference
  pBLEScan->setInterval(2000);    // 增加扫描间隔，减少对WiFi的干扰
  pBLEScan->setWindow(200);       // 减少扫描窗口，降低资源占用
  pBLEScan->setActiveScan(false); // 使用被动扫描，减少功耗和干扰
  pBLEScan->start(3, false);      // 减少扫描时间到3秒
}

void PetoiBtStopScan() {
  // Stop the BLE scan
  BLEScan* pBLEScan = BLEDevice::getScan();
  if (pBLEScan != nullptr) {
    pBLEScan->stop();
    pBLEScan->clearResults();  // Clear scan results
    pBLEScan->setAdvertisedDeviceCallbacks(nullptr);  // Remove callback
  }
  
  // Delete the callback instance and clean up memory
  if (pAdvertisedDeviceCallbacks != nullptr) {
    delete pAdvertisedDeviceCallbacks;
    pAdvertisedDeviceCallbacks = nullptr;
  }
  
  // Clean up BLE client resources
  if (pBleClient != nullptr) {
    if (btConnected) {
      pBleClient->disconnect();
    }
    delete pBleClient;
    pBleClient = nullptr;
  }
  
  // Clean up client callback
  if (pClientCallback != nullptr) {
    delete pClientCallback;
    pClientCallback = nullptr;
  }
  
  // Reset connection state
  btConnected = false;
  btReceiveDone = false;
  btRxLoad = "";
  
  // Reset scan-related flags
  doScan = false;
  doConnect = false;
  
  // Clean up the advertised device pointer if it exists
  if (PetoiBtDevice != nullptr) {
    delete PetoiBtDevice;
    PetoiBtDevice = nullptr;
  }
  
  // Clear remote characteristics pointers (they will be invalid after disconnect)
  pRemoteCharacteristicTx = nullptr;
  pRemoteCharacteristicRx = nullptr;
  pRemoteCharacteristicTemp = nullptr;
  pRD = nullptr;
  
  PTLF("BLE scan stopped and all resources cleaned up");
}

void checkBtScan() {
  if (doConnect) {
    if (connectToServer()) {
      String bleMessage = String(MODEL) + '\n';
      pRemoteCharacteristicTx->writeValue(bleMessage.c_str(), bleMessage.length()); //tell the Bit model name
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }
}

void bleClientSetup() {
  PTLF("Start...");
  // serverBtDeviceName = "BBC micro:bit [vatip]";  // It should be modified according to your own board.  another one's name: pogiv
  PetoiBtStartScan();
}

void readBleClient() {
  checkBtScan();
  if (btConnected && btReceiveDone && btRxLoad.length() > 0) {
    bleParser(btRxLoad);
    btRxLoad = "";
  }
}
