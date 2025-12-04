/* Write and read parameters to the permanent memory.
   Maximum bytes of I2C EEPROM is 65536 bit. i.e. address stops at 65535.
   Extra data will wrap over to address 0

   If there's no I2C EEPROM, use ESP32's flash to simulate the EEPROM.

   Rongzhong Li
   September 2024

   Copyright (c) 2024 Petoi LLC.

  The MIT license

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <Preferences.h>
Preferences config;

#define WIRE_BUFFER 30  // Arduino wire allows 32 byte buffer, with 2 byte for address.
#define WIRE_LIMIT 16   // That leaves 30 bytes for data. use 16 to balance each writes
#define PAGE_LIMIT 32   // AT24C32D 32-byte Page Write Mode. Partial Page Writes Allowed
#define SIZE (65535 / 8)
#define EEPROM_SIZE (65535 / 8)
bool EEPROMOverflow = false;

#define EEPROM_BIRTHMARK_ADDRESS 1     // avoid address 0, offset by 1. Using 0 will randomly reset the value when recalibrating IMU.
#define EEPROM_MPU 2                   // 2x9 = 18 bytes
#define EEPROM_ICM 20                  // 6x4 = 24 bytes (float values for ICM42670)
#define EEPROM_CALIB 45                // 16 bytes
#define EEPROM_DEVICE_NAME 61          // 20 bytes (shared by BLE, SSP, and WiFi)
#define EEPROM_BOOTUP_SOUND_STATE 81   // 1 byte
#define EEPROM_BUZZER_VOLUME 82        // 1 byte
#define EEPROM_MODULE_ENABLED_LIST 83  // 9 bytes
#define EEPROM_VERSION_DATE 95         // 11 bytes
#define EEPROM_DEFAULT_LAN 107         // 1 byte. 0: English, 1: Chinese
#define EEPROM_CURRENT_LAN 108         // 1 byte. 0: English, 1: Chinese
#define EEPROM_WIFI_MANAGER 109        // 1 byte. 0: don't launch, 1: launch wifi manager
#define EEPROM_RESERVED 110            // reserved for future use
#define SERIAL_BUFF 120                // needs to be after all EEPROM definitions

int dataLen(int8_t p) {
  byte skillHeader = p > 0 ? 4 : 7;
  int frameSize = p > 1 ? WALKING_DOF :  // gait
                      p == 1 ? DOF
                             :  // posture
                      DOF + 4;  // behavior
  int len = skillHeader + abs(p) * frameSize;
  return len;
}

void i2cDetect(TwoWire &wirePort) {
  if (&wirePort == &Wire1)
    wirePort.begin(UART_TX2, UART_RX2, 400000);
  byte error, address;
  int nDevices;
  int8_t i2cAddress[] = {0x39, 0x50, 0x54, 0x60, 0x62, 0x68, 0x69};
  String i2cAddressName[] = {"APDS9960 Gesture", "Mu3 CameraP", "EEPROM",
#ifdef SENTRY2_CAMERA
                             "Sentry2 Camera",
#else
                             "Mu3 Camera",
#endif
                             "AI Vision",        "MPU6050",     "ICM42670"};
  Serial.println("Scanning I2C network...");
  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    wirePort.beginTransmission(address);
    error = wirePort.endTransmission();
    if (error == 0) {
      Serial.print("- I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(":\t");
      for (byte i = 0; i < sizeof(i2cAddress) / sizeof(int8_t); i++) {
        if (address == i2cAddress[i]) {
          PT(i2cAddressName[i]);
          if (i == 1)
            MuQ = true;
          else if (i == 2)
            eepromQ = true;
          else if (i == 3)
#ifdef SENTRY2_CAMERA
            Sentry2Q = true;
#else
            MuQ = true;  // The older Mu3 Camera and Sentry1 share the same address. Sentry is not supported yet.
#endif
          else if (i == 4)
            GroveVisionQ = true;
#ifdef IMU_MPU6050
          else if (i == 5)
            mpuQ = true;
#endif
#ifdef IMU_ICM42670
          else if (i == 6)
            icmQ = true;
#endif
          nDevices++;
          break;
        }
        if (i == sizeof(i2cAddress) / sizeof(int8_t) - 1) {
          PT("Misc.");
        }
      }
      PTL();
    } else if (error == 4) {
      Serial.print("- Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (!icmQ && !mpuQ) {
    updateGyroQ = false;
    PTL("No IMU detected!");
  }
  if (nDevices == 0)
    Serial.println("- No I2C devices found");
  else
    Serial.println("- done");
  if (&wirePort == &Wire1)
    wirePort.end();
  PTHL("GroveVisionQ", GroveVisionQ);
#ifdef SENTRY2_CAMERA
  PTHL("Sentry2Q", Sentry2Q);
#else
  PTHL("MuQ", MuQ);
#endif
}

#ifdef I2C_EEPROM_ADDRESS
void i2c_eeprom_write_byte(unsigned int eeaddress, byte data) {
  // wait for other I2C devices to release the bus
  while (imuLockI2c || gestureLockI2c)
    delay(1);
  eepromLockI2c = true;  // lock I2C bus for EEPROM operations
  
  int rdata = data;
  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.write(rdata);
  Wire.endTransmission();
  delay(5);  // needs 5ms for write
  
  eepromLockI2c = false;  // release I2C bus lock
}

byte i2c_eeprom_read_byte(unsigned int eeaddress) {
  // wait for other I2C devices to release the bus
  while (imuLockI2c || gestureLockI2c)
    delay(1);
  eepromLockI2c = true;  // lock I2C bus for EEPROM operations
  
  byte rdata = 0xFF;
  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.endTransmission();
  Wire.requestFrom(I2C_EEPROM_ADDRESS, 1);
  if (Wire.available())
    rdata = Wire.read();
    
  eepromLockI2c = false;  // release I2C bus lock
  return rdata;
}

// This function will write a 2-byte integer to the EEPROM at the specified address and address + 1
void i2c_eeprom_write_int16(unsigned int eeaddress, int16_t p_value) {
  // wait for other I2C devices to release the bus
  while (imuLockI2c || gestureLockI2c)
    delay(1);
  eepromLockI2c = true;  // lock I2C bus for EEPROM operations
  
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);
  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.write(lowByte);
  Wire.write(highByte);
  Wire.endTransmission();
  delay(5);  // needs 5ms for write
  
  eepromLockI2c = false;  // release I2C bus lock

  //  EEPROM.update(p_address, lowByte);
  //  EEPROM.update(p_address + 1, highByte);
}

// This function will read a 2-byte integer from the EEPROM at the specified address and address + 1
int16_t i2c_eeprom_read_int16(unsigned int eeaddress) {
  // wait for other I2C devices to release the bus
  while (imuLockI2c || gestureLockI2c)
    delay(1);
  eepromLockI2c = true;  // lock I2C bus for EEPROM operations
  
  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.endTransmission();
  Wire.requestFrom(I2C_EEPROM_ADDRESS, 2);
  byte lowByte = Wire.read();
  byte highByte = Wire.read();
  
  eepromLockI2c = false;  // release I2C bus lock
  return (int16_t((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00));
}

// This function will write a 4-byte float to the EEPROM at the specified address
void i2c_eeprom_write_float(unsigned int eeaddress, float value) {
  // wait for other I2C devices to release the bus
  while (imuLockI2c || gestureLockI2c)
    delay(1);
  eepromLockI2c = true;  // lock I2C bus for EEPROM operations
  
  union {
    float f;
    byte bytes[4];
  } floatUnion;
  floatUnion.f = value;

  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  for (int i = 0; i < 4; i++) {
    Wire.write(floatUnion.bytes[i]);
  }
  Wire.endTransmission();
  delay(5);  // needs 5ms for write
  
  eepromLockI2c = false;  // release I2C bus lock
}

// This function will read a 4-byte float from the EEPROM at the specified address
float i2c_eeprom_read_float(unsigned int eeaddress) {
  // wait for other I2C devices to release the bus
  while (imuLockI2c || gestureLockI2c)
    delay(1);
  eepromLockI2c = true;  // lock I2C bus for EEPROM operations
  
  union {
    float f;
    byte bytes[4];
  } floatUnion;

  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.endTransmission();
  Wire.requestFrom(I2C_EEPROM_ADDRESS, 4);
  for (int i = 0; i < 4; i++) {
    if (Wire.available())
      floatUnion.bytes[i] = Wire.read();
  }
  
  eepromLockI2c = false;  // release I2C bus lock
  return floatUnion.f;
}

void i2c_eeprom_read_buffer(unsigned int eeaddress, byte *buffer, int length) {
  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.endTransmission();
  Wire.requestFrom(I2C_EEPROM_ADDRESS, length);
  int c = 0;
  for (c = 0; c < length; c++) {
    if (Wire.available())
      buffer[c] = Wire.read();
    //    PT((char)buffer[c]);
  }
}

void writeLong(unsigned int eeAddress, char *data, int len) {
  // byte locationInPage = eeAddress % PAGE_LIMIT;
  if (eeAddress + len >= SIZE) {
    PTL();
    PTL("EEPROM overflow!\n");
    beep(10, 100, 100, 5);
    return;
  }
  i2c_eeprom_write_byte(eeAddress++, len);
  //  PTL("write " + String(len) + " bytes");
  int writtenToEE = 0;
  while (len > 0) {
    Wire.beginTransmission(I2C_EEPROM_ADDRESS);
    Wire.write((int)((eeAddress) >> 8));  // MSB
    Wire.write((int)((eeAddress) & 0xFF));  // LSB
    //    PT("* current address: ");
    //    PT((unsigned int)eeAddress);
    //    PTL("\t0 1 2 3 4 5 6 7 8 9 a b c d e f ");
    //    PT("\t\t\t\t");
    byte writtenToWire = 0;
    do {
      //      PT(data[writtenToEE]);
      //      PT(" ");
      Wire.write((byte)data[writtenToEE]);
      writtenToWire++;
      writtenToEE++;
      eeAddress++;
      len--;
    } while (len > 0 && (eeAddress % PAGE_LIMIT) && writtenToWire < WIRE_LIMIT);
    Wire.endTransmission();
    delay(6);  // needs 5ms for page write
    //    PTL();
    //    PTL("wrote " + String(writtenToWire) + " bytes.");
  }
  //  PTL("finish writing");
}

void readLong(unsigned int eeAddress, char *data) {
  int len = i2c_eeprom_read_byte(eeAddress++);
  PTL("read " + String(len) + " bytes");
  int readFromEE = 0;
  int readToWire = 0;

  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)((eeAddress) >> 8));  // MSB
  Wire.write((int)((eeAddress) & 0xFF));  // LSB
  Wire.endTransmission();
  while (len > 0) {
    Wire.requestFrom(I2C_EEPROM_ADDRESS, min(WIRE_BUFFER, len));
    readToWire = 0;
    do {
      if (Wire.available())
        data[readFromEE] = Wire.read();
      PT((char)data[readFromEE]);
      readFromEE++;
    } while (--len > 0 && ++readToWire < WIRE_BUFFER);
    PTL();
  }
  PTL("finish reading");
}

char *readLongByBytes(int address) {
  int len = i2c_eeprom_read_byte(address);

  // Check for invalid length (uninitialized EEPROM or corrupted data)
  if (len < 0 || len > 50 || len == 255) {
    // Return NULL to indicate invalid data, caller should handle this
    return NULL;
  }

  char *id = new char[len + 1];
  if (id == NULL) {
    return NULL;
  }

  for (int i = 0; i < len; i++) {
    id[i] = i2c_eeprom_read_byte(address + 1 + i);
  }
  id[len] = '\0';
  return id;
}

void copydataFromBufferToI2cEeprom(unsigned int eeAddress, int8_t *newCmd) {
  int len = dataLen(newCmd[0]) + 1;
  if (eeAddress + len >= EEPROM_SIZE) {
    PTL();
    PTLF("I2C EEPROM overflow! Delete some skills!\n");
    EEPROMOverflow = true;
#ifdef BUZZER
    beep(10, 100, 100, 2);
#endif
    return;
  }
  int writtenToEE = 0;
  while (len > 0) {
    Wire.beginTransmission(I2C_EEPROM_ADDRESS);
    Wire.write((int)((eeAddress) >> 8));  // MSB
    Wire.write((int)((eeAddress) & 0xFF));  // LSB
    byte writtenToWire = 0;
    do {
      Wire.write((byte)newCmd[writtenToEE++]);
      writtenToWire++;
      eeAddress++;
    } while ((--len > 0) && (eeAddress % PAGE_LIMIT)
             && (writtenToWire < WIRE_LIMIT));  // be careful with the chained conditions
    // self-increment may not work as expected
    Wire.endTransmission();
    delay(6);  // needs 5ms for page write
    //    PTL("\nwrote " + String(writtenToWire) + " bytes.");
  }
  delay(6);
  //  PTLF("finish copying to I2C EEPROM");
}
void loadDataFromI2cEeprom(unsigned int eeAddress) {
  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)((eeAddress) >> 8));  // MSB
  Wire.write((int)((eeAddress) & 0xFF));  // LSB
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)I2C_EEPROM_ADDRESS, (uint8_t)1);
  newCmd[0] = Wire.read();
  int bufferLen = dataLen(newCmd[0]);
  //      int tail = bufferLen;
  int readFromEE = 0;
  int readToWire = 0;
  while (bufferLen > 0) {
    // PTL("request " + String(min(WIRE_BUFFER, len)));
    Wire.requestFrom((uint8_t)I2C_EEPROM_ADDRESS, (uint8_t)min(WIRE_BUFFER, bufferLen));
    readToWire = 0;
    do {
      if (Wire.available())
        newCmd[1 + readFromEE++] = Wire.read();
      //      PT( (int8_t)newCmd[readFromEE - 1]);
      //      PT('\t');
    } while (--bufferLen > 0 && ++readToWire < WIRE_BUFFER);
    //    PTL();
  }
  //      newCmd[tail] = '\0';
}

// This function will write a 2-byte unsigned integer to the EEPROM at the specified address and address + 1
// Used specifically for storing EEPROM addresses which range from 0-65535 and must be unsigned
// to avoid negative value issues when address > 32767
void i2c_eeprom_write_uint16(unsigned int eeaddress, uint16_t p_value) {
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);
  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.write(lowByte);
  Wire.write(highByte);
  Wire.endTransmission();
  delay(5);  // needs 5ms for write
}

// This function will read a 2-byte unsigned integer from the EEPROM at the specified address and address + 1
// Critical for reading EEPROM addresses correctly - prevents negative values when address > 32767
// which would cause invalid memory access when cast to unsigned int
uint16_t i2c_eeprom_read_uint16(unsigned int eeaddress) {
  Wire.beginTransmission(I2C_EEPROM_ADDRESS);
  Wire.write((int)(eeaddress >> 8));    // MSB
  Wire.write((int)(eeaddress & 0xFF));  // LSB
  Wire.endTransmission();
  Wire.requestFrom(I2C_EEPROM_ADDRESS, 2);
  byte lowByte = Wire.read();
  byte highByte = Wire.read();
  return (uint16_t((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00));
}

#endif

bool newBoardQ(unsigned int eeaddress = EEPROM_BIRTHMARK_ADDRESS) {
// PTHL("birthmark:", char(i2c_eeprom_read_byte(eeaddress)));
#ifdef I2C_EEPROM_ADDRESS
  // Use I2C lock to protect BIRTHMARK reading, avoid conflicts with other I2C operations
  while (imuLockI2c || gestureLockI2c) delay(1);
  eepromLockI2c = true;
  byte birthmarkValue = i2c_eeprom_read_byte(eeaddress);
  eepromLockI2c = false;
  return birthmarkValue != BIRTHMARK;
#else
  return config.getChar("birthmark") != BIRTHMARK;
#endif
}

void resetAsNewBoard(char mark) {
#ifdef I2C_EEPROM_ADDRESS
  i2c_eeprom_write_byte(EEPROM_BIRTHMARK_ADDRESS, mark);  // esp_random() % 128); //mark the board as uninitialized
#else
  config.putChar("birthmark", mark);
#endif
  PTL("Alter the birthmark for reset!");
  delay(5);
  ESP.restart();
}

char data[] =
    " The quick brown fox jumps over the lazy dog. \
The five boxing wizards jump quickly. Pack my box with five dozen liquor jugs.";  // data to write

// char data[]={16,-3,5,7,9};

void genBleID(int suffixDigits = 2) {
  const char *prefix =
#ifdef BITTLE
      "Bittle"
#elif defined NYBBLE
      "Nybble"
#elif defined CPT
      "Cpt"
#else
      "Cub"
#endif
      ;
  int prelen = strlen(prefix);

  char *id = new char[prelen + suffixDigits + 1];
  strcpy(id, prefix);
  for (int i = 0; i < suffixDigits; i++) {
    int temp = esp_random() % 16;
    sprintf(id + prelen + i, "%X", temp);
  }
  id[prelen + suffixDigits] = '\0';

#ifdef I2C_EEPROM_ADDRESS
      writeLong(EEPROM_DEVICE_NAME, id, prelen + suffixDigits);
    char *temp = readLongByBytes(EEPROM_DEVICE_NAME);
  if (temp != NULL) {
    uniqueName = String(temp);
    delete[] temp;
  } else {
    // EEPROM data is invalid (probably due to address change), use the generated ID
    uniqueName = String(id);
  }
#else
  config.putString("ID", id);
  uniqueName = String(id);
#endif
  PTL(uniqueName);
  delete[] id;
}

void customBleID(char *customName, int8_t len) {
#ifdef I2C_EEPROM_ADDRESS
      writeLong(EEPROM_DEVICE_NAME, customName, len + 1);
#else
  config.putString("ID", customName);
#endif
}

// Get device name with specified suffix using global uniqueName
// Returns a dynamically allocated string that must be freed by caller
char *getDeviceName(const char *suffix) {
  String deviceName = uniqueName + suffix;
  char *result = new char[deviceName.length() + 1];
  strcpy(result, deviceName.c_str());
  return result;
}

void resetIfVersionOlderThan(String versionStr) {
#ifdef I2C_EEPROM_ADDRESS
  char *savedVersionDate = readLongByBytes(EEPROM_VERSION_DATE);

  String savedVersionStr = (savedVersionDate && strlen(savedVersionDate) > 0) ? String(savedVersionDate) : "unknown";
  long savedDate =
      (savedVersionDate && strlen(savedVersionDate) >= 6) ? atoi(savedVersionDate + strlen(savedVersionDate) - 6) : 0;
  if (savedVersionDate)
    delete[] savedVersionDate;
#else
  String savedVersionStr = config.getString("versionDate", "unknown");
  long savedDate = (savedVersionStr == "unknown") ? 0 : savedVersionStr.substring(savedVersionStr.length() - 6).toInt();
#endif
  long currentDate = atol(versionStr.c_str() + versionStr.length() - 6);
  if (savedDate < currentDate) {
    delay(1000);
    PTTL("\n* The previous version on the board is ", savedVersionStr);
    PTTL("* The robot will reboot and upgrade to ", versionStr);
    resetAsNewBoard('X');
  }
}

void configSetup() {
  if (newBoard) {
    PTLF("Set up the new board...");
    char tempStr[12];
    strcpy(tempStr, SoftwareVersion.c_str());
    soundState = 1;
    buzzerVolume = 5;
    PTLF("Unmute and set volume to 5/10");

    int bufferLen = dataLen(rest[0]);  // save a preset skill to the temp skill
    arrayNCPY(newCmd, rest, bufferLen);
    PTF("- Name the new robot as: ");
#ifdef BT_BLE
    genBleID();
#endif

#ifdef I2C_EEPROM_ADDRESS
    PTL("Using constants from I2C EEPROM");
    writeLong(EEPROM_VERSION_DATE, tempStr, SoftwareVersion.length());
    i2c_eeprom_write_byte(EEPROM_BOOTUP_SOUND_STATE, soundState);
    i2c_eeprom_write_byte(EEPROM_BUZZER_VOLUME, buzzerVolume);
    for (byte i = 0; i < sizeof(moduleList) / sizeof(char); i++)
      i2c_eeprom_write_byte(EEPROM_MODULE_ENABLED_LIST + i, moduleActivatedQ[i]);
    i2c_eeprom_write_byte(EEPROM_DEFAULT_LAN, 'a');  // a for English, b for Chinese
    i2c_eeprom_write_byte(EEPROM_CURRENT_LAN, 'b');  // a for English, b for Chinese
    // save a preset skill to the temp skill in case its called before assignment
    unsigned int i2cEepromAddress = SERIAL_BUFF + 2;  // + esp_random() % (EEPROM_SIZE - SERIAL_BUFF - 2 - 2550); //save
                                                      // to random position to protect the EEPROM
    // Use uint16 version to properly handle addresses > 32767 without sign extension issues
    i2c_eeprom_write_uint16(SERIAL_BUFF, (uint16_t)i2cEepromAddress);  // the address takes 2 bytes to store
    copydataFromBufferToI2cEeprom(i2cEepromAddress, (int8_t *)newCmd);
    i2c_eeprom_write_byte(EEPROM_WIFI_MANAGER, rebootForWifiManagerQ);

#else
    PTL("Using constants from on-board Flash");
    config.putString("versionDate", tempStr);
    config.putBool("bootSndState", soundState);
    config.putChar("buzzerVolume", buzzerVolume);
    config.putBytes("moduleState", moduleActivatedQ, sizeof(moduleList) / sizeof(char));
    config.putChar("defaultLan", 'a');  // a for English, b for Chinese
    config.putChar("currentLan", 'b');  // a for English, b for Chinese
    // save a preset skill to the temp skill in case its called before assignment
    config.putInt("tmpLen", bufferLen);
    config.putBytes("tmp", (int8_t *)newCmd, bufferLen);
    config.putBool("WifiManager", rebootForWifiManagerQ);  // default is false
#endif
#ifndef AUTO_INIT
#ifdef VOLTAGE
    if (!lowBatteryQ)  // won't play sound if only powered by USB. It avoid noise when developing codes
#endif
      playMelody(melodyInit, sizeof(melodyInit) / 2);
#endif
#ifndef AUTO_INIT
    PTL("- Reset the joints' calibration offsets? (Y/n): ");
    char choice = getUserInputChar();
    PTL(choice);
    if (choice == 'Y' || choice == 'y') {
#else
    PTL("- Reset the joints' calibration offsets...");
#endif
#ifdef I2C_EEPROM_ADDRESS
      for (byte c = 0; c < DOF; c++)
        i2c_eeprom_write_byte(EEPROM_CALIB + c, 0);
#else
    config.putBytes("calib", servoCalib, DOF);
#endif
#ifndef AUTO_INIT
    }
#endif
  } else {
    resetIfVersionOlderThan(SoftwareVersion);
#ifdef I2C_EEPROM_ADDRESS
    soundState = i2c_eeprom_read_byte(EEPROM_BOOTUP_SOUND_STATE);
    buzzerVolume = max(byte(0), min(byte(10), i2c_eeprom_read_byte(EEPROM_BUZZER_VOLUME)));
    for (byte i = 0; i < sizeof(moduleList) / sizeof(char); i++)
      moduleActivatedQ[i] = i2c_eeprom_read_byte(EEPROM_MODULE_ENABLED_LIST + i);
    defaultLan = (char)i2c_eeprom_read_byte(EEPROM_DEFAULT_LAN);
    currentLan = (char)i2c_eeprom_read_byte(EEPROM_CURRENT_LAN);
    char *temp = readLongByBytes(EEPROM_DEVICE_NAME);
    if (temp != NULL) {
      uniqueName = String(temp);
      delete[] temp;
    } else {
      // EEPROM data is invalid, generate a new ID following normal naming rules
      const char *prefix =
#ifdef BITTLE
          "Bittle"
#elif defined NYBBLE
          "Nybble"
#else
          "Cub"
#endif
          ;
      int prelen = strlen(prefix);
      int suffixDigits = 2;

      char *newId = new char[prelen + suffixDigits + 1];
      if (newId != NULL) {
        strcpy(newId, prefix);
        for (int i = 0; i < suffixDigits; i++) {
          int temp = esp_random() % 16;
          sprintf(newId + prelen + i, "%X", temp);
        }
        newId[prelen + suffixDigits] = '\0';

        // Save the new ID to EEPROM
        writeLong(EEPROM_DEVICE_NAME, newId, prelen + suffixDigits);
        uniqueName = String(newId);
        delete[] newId;
      } else {
        // Memory allocation failed, use String-based fallback with random suffix
        String randomSuffix = "";
        for (int i = 0; i < suffixDigits; i++) {
          int temp = esp_random() % 16;
          String hexChar = String(temp, HEX);
          hexChar.toUpperCase();
          randomSuffix += hexChar;
        }
        uniqueName = String(prefix) + randomSuffix;
      }
    }
    rebootForWifiManagerQ = i2c_eeprom_read_byte(EEPROM_WIFI_MANAGER);

#else
    soundState = config.getBool("bootSndState") ? 1 : 0;
    buzzerVolume = max(byte(0), min(byte(10), (byte)config.getChar("buzzerVolume")));
    config.getBytes("moduleState", moduleActivatedQ, sizeof(moduleList) / sizeof(char));
    defaultLan = config.getChar("defaultLan");
    currentLan = config.getChar("currentLan");
    uniqueName = config.getString("ID", "P");
    rebootForWifiManagerQ = config.getBool("WifiManager");
    PT(config.freeEntries());                                 // show remaining entries of the preferences.
    PTL(" entries are available in the namespace table.\n");  // this method works regardless of the mode in which the
                                                              // namespace is opened.
#endif
    PTHL("Default language: ", defaultLan == 'b' ? " Chinese" : " English");
#ifdef VOLTAGE
    if (!lowBatteryQ)  // won't play sound if only powered by USB. It avoid noise when developing codes
#endif
      playMelody(melodyNormalBoot, sizeof(melodyNormalBoot) / 2);
  }
}

void saveCalib(int8_t *var) {
#ifndef I2C_EEPROM_ADDRESS
  config.putBytes("calib", var, DOF);
#endif
  for (byte s = 0; s < DOF; s++) {
#ifdef I2C_EEPROM_ADDRESS
    i2c_eeprom_write_byte(EEPROM_CALIB + s, var[s]);
#endif
    calibratedZeroPosition[s] = zeroPosition[s] + float(var[s]) * rotationDirection[s];
  }
}

// clang-format off
// Forward Declarations
bool listEspPartitions();
bool listUniqueNvsNamespaces();
bool listNamespacesWithKeysAndValues(const char *partition_label);
bool listKeysAndValues(const char *partition_label, const char *namespace_name);

void displayNsvPartition()
{
/*  Created by este este - 28-MAR-2025
      @ Lists all partitions on the ESP32 chip.
      @ If the default 'nsv' partition is found then any namespaces in that partition are listed.
      @ For each namespace in the default 'nsv' partition, the key-value pairs are listed.
*/

/*  The ESP32 Non-Volatile Storage (NVS) system supports the following value types for storing data via the nvs_type_t enum which has the following values:
    NVS_TYPE_U8: Unsigned 8-bit integer.
    NVS_TYPE_I8: Signed 8-bit integer.
    NVS_TYPE_U16: Unsigned 16-bit integer.
    NVS_TYPE_I16: Signed 16-bit integer.
    NVS_TYPE_U32: Unsigned 32-bit integer.
    NVS_TYPE_I32: Signed 32-bit integer.
    NVS_TYPE_U64: Unsigned 64-bit integer.
    NVS_TYPE_I64: Signed 64-bit integer.
    NVS_TYPE_STR: Null-terminated string.
    NVS_TYPE_BLOB: Binary large object (arbitrary binary data).
    NVS_TYPE_ANY: A special type used during iteration to indicate that any type of entry is acceptable.
*/

  if ( !listEspPartitions() )
    {
      return;
    }

  if ( !listUniqueNvsNamespaces() )
    {
      return;
    }

  if ( !listNamespacesWithKeysAndValues("nvs") )
    {
      return;
    }
}

bool listEspPartitions()
{
  // Created by este este

  /* <<<<< FIND ALL PARTITIONS >>>>> */

  bool defaultNvsPartitionFoundQ = false;

  // Tag for logging
  const char *TAG __attribute__((unused)) = "PARTITIONS";

  // Iterator to find all partitions
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if ( it == NULL )
    {
      ESP_LOGI(TAG, "No partitions found");
      Serial.println("No partitions found.");
      return defaultNvsPartitionFoundQ;
    }

  Serial.println("\nLocating ALL Partitions...\n");

  // Iterate through all partitions
  while ( it != NULL )
    {
      const esp_partition_t *partition = esp_partition_get(it);

      // Print partition details
      ESP_LOGI(TAG, "Found Partition:");
      ESP_LOGI(TAG, "Label: %s, Address: 0x%X, Size: %d bytes, Type: %d, Subtype: %d", partition->label, partition->address, partition->size, partition->type, partition->subtype);

      Serial.printf("Found Partition:\n");
      Serial.printf("\tLabel: %s\n", partition->label);

      if ( partition->subtype == ESP_PARTITION_SUBTYPE_DATA_NVS )
        {
          Serial.printf("     Partition labeled '%s' is an NVS partition.\n", partition->label);
          if ( String(partition->label) == "nvs" )
            {
              defaultNvsPartitionFoundQ = true;
            }
        }

      Serial.printf("\tAddress: 0x%X\n", partition->address);
      Serial.printf("\tSize: %d bytes\n", partition->size);
      Serial.printf("\tType: %d\n", partition->type);
      Serial.printf("\tSubtype: %d\n", partition->subtype);
      Serial.println(); // Add a blank line for readability

      // Move to the next partition
      it = esp_partition_next(it);
    }

  // Release the iterator
  esp_partition_iterator_release(it);

  if ( !defaultNvsPartitionFoundQ )
    {
      Serial.printf("\nDefault 'nvs' partition was NOT found so exiting.\n");
      return defaultNvsPartitionFoundQ;
    }
  else
    {
      defaultNvsPartitionFoundQ = true;
      Serial.printf("\nDefault 'nvs' partition WAS found so continuing.\n");
      return defaultNvsPartitionFoundQ;
    }
}

bool listUniqueNvsNamespaces()
{
  // Created by este este

  bool defaultNvsNameSpaceFoundQ = false;

  // Initialize default NVS partition 'nvs'
  esp_err_t err = nvs_flash_init();  // only works for the default partition.
  if ( err != ESP_OK )
    {
      Serial.printf("Failed to initialize NVS partition with default name of 'nvs': %s\n", esp_err_to_name(err));
      return defaultNvsNameSpaceFoundQ;
    }

  // Create an iterator for entries in NVS
  nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, NULL, NVS_TYPE_ANY);
  if ( it == NULL )
    {
      Serial.println("\nNo namespaces found.");
      return defaultNvsNameSpaceFoundQ;
    }

  std::set<String> uniqueNamespaces;  // To store unique namespace names
  Serial.println("\nNamespaces in the default 'nvs' partition:");
  while ( it != NULL )
    {
      defaultNvsNameSpaceFoundQ = true;
      nvs_entry_info_t info;
      nvs_entry_info(it, &info);

      // Add unique namespaces to the set
      if ( uniqueNamespaces.find(String(info.namespace_name)) == uniqueNamespaces.end() )
        {
          uniqueNamespaces.insert(String(info.namespace_name));
          Serial.printf("- Namespace: %s\n", info.namespace_name);
        }

      it = nvs_entry_next(it);
    }
  return defaultNvsNameSpaceFoundQ;
}

bool listNamespacesWithKeysAndValues(const char *partition_label)
{
  // Created by este este

  bool             successQ = false;

  std::set<String> uniqueNamespaces;  // Store unique namespace names
  nvs_iterator_t   it = nvs_entry_find(partition_label, NULL, NVS_TYPE_ANY);
  if ( it == NULL )
    {
      Serial.printf("\nNo namespaces found in partition '%s'\n", partition_label);
      return successQ;
    }

  while ( it != NULL )
    {
      successQ = true;
      nvs_entry_info_t info;
      nvs_entry_info(it, &info);

      // Add to unique set and process if not already listed
      if ( uniqueNamespaces.find(String(info.namespace_name)) == uniqueNamespaces.end() )
        {
          uniqueNamespaces.insert(String(info.namespace_name));
          Serial.printf("\nNamespace: %s\n", info.namespace_name);
          listKeysAndValues(partition_label, info.namespace_name);
        }
      it = nvs_entry_next(it);
    }
  return successQ;
}

bool listKeysAndValues(const char *partition_label, const char *namespace_name)
{
  // Created by este este

  bool         successQ = false;
  nvs_handle_t handle;
  esp_err_t    err;

  // Open the namespace in the specified partition
  err = nvs_open_from_partition(partition_label, namespace_name, NVS_READONLY, &handle);
  if ( err != ESP_OK )
    {
      Serial.printf("\nFailed to open namespace '%s' in partition '%s'\n", namespace_name, partition_label);
      return successQ;
    }

  Serial.printf("Keys and values in namespace '%s':\n", namespace_name);

  nvs_iterator_t it = nvs_entry_find(partition_label, namespace_name, NVS_TYPE_ANY);
  while ( it != NULL )
    {
      successQ = true;
      nvs_entry_info_t info;
      nvs_entry_info(it, &info);

      Serial.printf("- Key: %s\n", info.key);

      // Handle all supported value types
      switch ( info.type )
        {
          case NVS_TYPE_U8:
            {
              uint8_t value;
              if ( nvs_get_u8(handle, info.key, &value) == ESP_OK )
                {
                  Serial.printf("  Value (uint8): %u\n", value);
                }
              break;
            }
          case NVS_TYPE_I8:
            {
              int8_t value;
              if ( nvs_get_i8(handle, info.key, &value) == ESP_OK )
                {
                  Serial.printf("  Value (int8): %d\n", value);
                }
              break;
            }
          case NVS_TYPE_U16:
            {
              uint16_t value;
              if ( nvs_get_u16(handle, info.key, &value) == ESP_OK )
                {
                  Serial.printf("  Value (uint16): %u\n", value);
                }
              break;
            }
          case NVS_TYPE_I16:
            {
              int16_t value;
              if ( nvs_get_i16(handle, info.key, &value) == ESP_OK )
                {
                  Serial.printf("  Value (int16): %d\n", value);
                }
              break;
            }
          case NVS_TYPE_U32:
            {
              uint32_t value;
              if ( nvs_get_u32(handle, info.key, &value) == ESP_OK )
                {
                  Serial.printf("  Value (uint32): %u\n", value);
                }
              break;
            }
          case NVS_TYPE_I32:
            {
              int32_t value;
              if ( nvs_get_i32(handle, info.key, &value) == ESP_OK )
                {
                  Serial.printf("  Value (int32): %d\n", value);
                }
              break;
            }
          case NVS_TYPE_U64:
            {
              uint64_t value;
              if ( nvs_get_u64(handle, info.key, &value) == ESP_OK )
                {
                  Serial.printf("  Value (uint64): %llu\n", value);
                }
              break;
            }
          case NVS_TYPE_I64:
            {
              int64_t value;
              if ( nvs_get_i64(handle, info.key, &value) == ESP_OK )
                {
                  Serial.printf("  Value (int64): %lld\n", value);
                }
              break;
            }
          case NVS_TYPE_STR:
            {
              size_t required_size = 0;
              nvs_get_str(handle, info.key, NULL, &required_size);
              char *value = (char *)malloc(required_size);
              if ( value != NULL && nvs_get_str(handle, info.key, value, &required_size) == ESP_OK )
                {
                  Serial.printf("  Value (string): %s\n", value);
                }
              free(value);
              break;
            }
          case NVS_TYPE_BLOB:
            {
              size_t required_size = 0;
              nvs_get_blob(handle, info.key, NULL, &required_size);
              uint8_t *blob = (uint8_t *)malloc(required_size);
              if ( blob != NULL && nvs_get_blob(handle, info.key, blob, &required_size) == ESP_OK )
                {
                  Serial.printf("  Value (blob): [size: %d bytes]\n", required_size);
                }
              free(blob);
              break;
            }
          default:
            Serial.printf("  Unsupported type\n");
            break;
        }
      it = nvs_entry_next(it);
    }
  nvs_close(handle);
  return successQ;
}
