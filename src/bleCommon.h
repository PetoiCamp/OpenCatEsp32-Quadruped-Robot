#ifndef BLE_COMMON_H
#define BLE_COMMON_H

// BLE UUID definitions - shared between BLE server and client
// See the following for generating UUIDs: https://www.uuidgenerator.net/
#define BLE_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"            // UART service UUID
#define BLE_CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // receive
#define BLE_CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // transmit

#endif // BLE_COMMON_H
