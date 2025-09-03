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

bool connectToServer() {
  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new btPetoiClientCallback());
  pClient->connect(PetoiBtDevice);
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    pClient->disconnect();
    return false;
  }
  pRemoteCharacteristicTx = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_TX);
  pRemoteCharacteristicRx = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID_RX);
  if (pRemoteCharacteristicTx == nullptr) {
    pClient->disconnect();
    return false;
  }
  if (pRemoteCharacteristicRx == nullptr) {
    pClient->disconnect();
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
      pClient->disconnect();
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

void PetoiBtStartScan() {
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new PetoiAdvertisedDeviceCallbacks());
  
  // Optimize scan parameters to improve connection stability
  pBLEScan->setInterval(2000);    // Increase scan interval, reduce interference
  pBLEScan->setWindow(1000);      // Increase scan window, improve discovery probability
  pBLEScan->setActiveScan(true);   // Keep active scanning
  pBLEScan->start(10, false);     // Increase scan time to 10 seconds
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
