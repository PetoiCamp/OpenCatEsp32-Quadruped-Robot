#include "soc/gpio_sig_map.h"

// — read master computer’s signals (middle level) —

// This example code is in the Public Domain (or CC0 licensed, at your option.)
// By Richard Li - 2020
//
// This example creates a bridge between Serial and Classical Bluetooth (SSP with authentication)
// and also demonstrate that SerialBT has the same functionalities as a normal Serial

template<typename T>
void printToAllPorts(T text, bool newLine = true) {
  String textResponse = String(text);
  if (newLine) {
    textResponse += "\r\n";
  }
#ifdef BT_BLE
  if (deviceConnected) {
    for (int i = 0; i < textResponse.length(); i += 10) {
      int endIndex = min(i + 10, (int)textResponse.length());
      bleWrite(textResponse.substring(i, endIndex));
    }
  } 
#endif
#ifdef BT_SSP
  if (BTconnected) {
    SerialBT.print(textResponse);
  }
#endif
#ifdef WEB_SERVER
  if (cmdFromWeb) {
    if (textResponse != "=") {
      webResponse += textResponse;
    }
  }
#endif
  if (moduleActivatedQ[0]) { // serial2
    Serial2.print(textResponse);
  }

  PT(textResponse);
}
