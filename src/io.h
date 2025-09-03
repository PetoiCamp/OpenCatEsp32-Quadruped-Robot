#include "soc/gpio_sig_map.h"

// — read master computer’s signals (middle level) —

// This example code is in the Public Domain (or CC0 licensed, at your option.)
// By Richard Li - 2020
//
// This example creates a bridge between Serial and Classical Bluetooth (SSP with authentication)
// and also demonstrate that SerialBT has the same functionalities as a normal Serial

#ifdef BT_SSP
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

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

template<typename T>
void printToAllPorts(T text, bool newLine = true) {
#ifdef BT_BLE
  if (deviceConnected)
    bleWrite(String(text));
#endif
#ifdef BT_SSP
  if (BTconnected)
    SerialBT.print(text);
#endif
#ifdef WEB_SERVER
  if (cmdFromWeb) {
    if (String(text) != "=") {
      webResponse += String(text);
      if (newLine)
        webResponse += '\n';
    }
  }
#endif
  if (moduleActivatedQ[0])  // serial2
    Serial2.println(text);
  PT(text);
  if (newLine)
  {
    PTL();
#ifdef BT_SSP
    if (BTconnected)
      SerialBT.println();
#endif
  }
}
