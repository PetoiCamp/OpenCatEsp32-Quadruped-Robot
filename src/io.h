#include "soc/gpio_sig_map.h"

// — read master computer’s signals (middle level) —

// This example code is in the Public Domain (or CC0 licensed, at your option.)
// By Richard Li - 2020
//
// This example creates a bridge between Serial and Classical Bluetooth (SSP with authentication)
// and also demonstrate that SerialBT has the same functionalities as a normal Serial

template<typename T>
void printToAllPorts(T text, bool newLine = true) {
#ifdef BT_BLE
  if (deviceConnected)
    bleWrite(String(text));
#endif
#ifdef BT_SSP
  if (BTconnected) {
    SerialBT.print(text);
    if (newLine)
      SerialBT.println();
  }
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
  if (moduleActivatedQ[0]) { // serial2
    Serial2.print(text);
    if (newLine)
    {
      Serial2.println();
    }
  }

  PT(text);
  if (newLine)
  {
    PTL();
  }
}
