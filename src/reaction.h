#include "esp32-hal.h"
#include <stdio.h>

// Async web server function declarations
#ifdef WEB_SERVER
void completeWebTask();
void errorWebTask(String errorMessage);
void finishWebCommand();
#endif

void dealWithExceptions() {
#ifdef GYRO_PIN

  // Handle turning exception regardless of gyroBalanceQ status
  if (imuException == IMU_EXCEPTION_TURNING) {
    PTL("EXCEPTION: turning target reached");
    // Stop the robot and make it stand up
    tQueue->addTask('k', "up");
    needTurning = false;  // Reset flag after creating task
    PTL("endTurn");
    prev_imuException = imuException;
    print6Axis();
  }
  
  // Handle other IMU exceptions only when gyroBalanceQ is true
  if (gyroBalanceQ) {
    // if (imuException == IMU_EXCEPTION_FLIPPED || (skill->period == 1 && fabs(xyzReal[2]) >= 15) || (skill->period > 1
    // && abs(xyzReal[2]) >= 20)) {
    //   delay(50);
    // }
    if (imuException && imuException != IMU_EXCEPTION_TURNING) {  // the gyro reaction switch can be toggled on/off by the 'g' token
      switch (imuException) {
        case IMU_EXCEPTION_LIFTED:
          {
            if (skill->period == 1 && prev_imuException != IMU_EXCEPTION_LIFTED) {
              if (ypr[1] < -50)
                strcpy(newCmd, "lifted");
              else
                strcpy(newCmd, "dropped");
              loadBySkillName(newCmd);
              token = 'k';
            }
            break;
          }
        case IMU_EXCEPTION_FREEFALL:
          {
            PTL("EXCEPTION free fall");
            strcpy(newCmd, "lnd");
            loadBySkillName(newCmd);
            shutServos();  // does not shut the P1S servo.while token p in serial monitor can.? ? ?
            delay(1000);
            token = 'k';
            strcpy(newCmd, "up");
            newCmdIdx = -1;
            break;
          }
        case IMU_EXCEPTION_FLIPPED:
          {
            PTL("EXCEPTION: Fall over");
            soundFallOver();
            //  for (int m = 0; m < 2; m++)
            //    meow(30 - m * 12, 42 - m * 12, 20);
            token = 'k';
            manualHeadQ = false;
            strcpy(newCmd, "rc");
            newCmdIdx = -2;
            // tQueue->addTaskToFront('k', "rc");
            break;
          }
        case IMU_EXCEPTION_KNOCKED:
          {
            if (tQueue->cleared() && skill->period == 1) {
              PTL("EXCEPTION: Knocked");
              tQueue->addTask('k', "knock");
#if defined NYBBLE && defined ULTRASONIC
              if (!moduleActivatedQ[0]) {  // serial2 may be used to connect serial2 rather than the RGB ultraconic sensor
                int8_t clrRed[] = { 125, 0, 0, 0, 0, 126 };
                int8_t clrBlue[] = { 0, 0, 125, 0, 0, 126 };
                tQueue->addTask('C', clrRed, 1);
                tQueue->addTask('C', clrBlue);
              }
#endif
              tQueue->addTask('k', "up");
            }
            break;
          }
        case IMU_EXCEPTION_PUSHED:
          {
            PTL("EXCEPTION: Pushed");
            // Acceleration Real
            //      ^ head
            //        ^ x+
            //        |
            //  y+ <------ y-
            //        |
            //        | x-
            if (skill->period == 1 && strncmp(lastCmd, "vtF", 2)) {
              char xSymbol[] = { '^', 'v' };
              char ySymbol[] = { '<', '>' };
              char xDirection = xSymbol[sign(ARX) > 0];
              char yDirection = ySymbol[sign(ARY) > 0];
              float forceAngle = atan(float(fabs(ARX)) / ARY) * degPerRad;
              PT(fabs(ARX) > fabs(ARY) ? xDirection : yDirection);
              PTHL(" ForceAngle:", forceAngle);
              if (tQueue->cleared()) {
                if (xDirection == '^') {
                  // tQueue->addTask('i', yDirection == '<' ? "0 -75" : "0 75");
                  if (fabs(forceAngle) < 60)
                    // tQueue->addTask('i', yDirection == '<' ? "0 45" : "0 -45");
                    tQueue->addTask('k', yDirection == '<' ? "wkL" : "wkR", 700);
                  // tQueue->addTask('i', "");
                  else {
                    tQueue->addTask('k', "wkF", 700);
                    // tQueue->addTask('i', "");
                    tQueue->addTask('k', "bkF", 500);
                  }
                } else {
                  // tQueue->addTask('k', yDirection == '<' ? "bkR" : "bkL", 1000);
                  if (fabs(forceAngle) < 60)
                    tQueue->addTask('k', yDirection == '<' ? "wkR" : "wkL", 700);
                  else {
                    tQueue->addTask('k', "bkF", 500);
                    tQueue->addTask('k', "wkF", 700);
                  }
                }
              }
              tQueue->addTask('k', "up");
              delayPrevious = runDelay;
              runDelay = delayException;
              PTL();
            }
            break;
          }
        case IMU_EXCEPTION_OFFDIRECTION:
          {
            PTL("EXCEPTION: off direction");
            // char *currentGait = skill->skillName;  // it may not be gait
            // char gaitDirection = currentGait[strlen(currentGait) - 1];
            float yawDiff = int(ypr[0] - previous_ypr[0]) % 180;
            if (tQueue->cleared()) {
              if (skill->period <= 1 || !strcmp(skill->skillName, "vtF")) {  // not gait or stepping
                tQueue->addTask('k', yawDiff > 0 ? "vtR" : "vtL", round(fabs(yawDiff) * 15));
                // tQueue->addTask('k', "up", 100);
                delayPrevious = runDelay;
                runDelay = delayException;
              } else {
                // if (gaitDirection == 'L' || gaitDirection == 'R')   //turning gait
                previous_ypr[0] = ypr[0];
              }
              // else {
              //   if (gaitDirection == 'L' || gaitDirection == 'R') {  //turning gait
              //     previous_ypr[0] = ypr[0];
              //   } else {
              //     currentGait[strlen(currentGait) - 1] = yawDiff > 0 ? 'R' : 'L';
              //     PTL(currentGait);
              //     tQueue->addTask('k', currentGait, round(fabs(yawDiff) * 15));
              //     currentGait[strlen(currentGait) - 1] = 'F';
              //     PTL(currentGait);
              //     tQueue->addTask('k', currentGait);
              //   }
              // }
            }
            break;
          }
        default:
          {
            break;
          }
      }
      // if (imuException != IMU_EXCEPTION_PUSHED)
      prev_imuException = imuException;
      print6Axis();
      // readIMU();  // flush the IMU to avoid static readings and infinite loop

      // if (tQueue->lastTask == NULL) {
      //   if (strcmp(lastCmd, "") && strcmp(lastCmd, "lnd") && *strGet(newCmd, -1) != 'L' && *strGet(lastCmd, -1) !=
      //   'R') {
      //     PTH("save last task ", lastCmd);
      //     tQueue->lastTask = new Task('k', lastCmd);
      //   }
      // }
    } else {
#ifndef ROBOT_ARM
      if (tQueue->cleared() && prev_imuException == IMU_EXCEPTION_LIFTED) {
        // strcpy(newCmd, "dropRec");
        // loadBySkillName(newCmd);
        // token = 'k';
        tQueue->addTask('k', "dropRec", 500);
      }
#endif
      prev_imuException = imuException;
    }
  }
  if (tQueue->cleared() && runDelay <= delayException)
    runDelay = delayMid;
#endif

#ifdef WEB_SERVER  // check if to reset the wifi manager and reboot
  if (digitalRead(0) == LOW) {
    delay(5);
    // PTLF("Debounce boot button.");
    if (digitalRead(0) != LOW)
      return;

#ifdef I2C_EEPROM_ADDRESS
    i2c_eeprom_write_byte(EEPROM_WIFI_MANAGER, true);
#else
    config.putBool("WifiManager", true);  // default is false
#endif
    PTLF("The robot will reboot and use Wifi manager.");
    PTLF("Hold the BOOT key and count down 10 if you want to clear the previous Wifi credentials.\n**********\n\n");
    int wifiCountdown = 10;
    while (digitalRead(0) == LOW && wifiCountdown) {
      delay(200);
      PT(wifiCountdown--);
      PT("..");
    }
    PTL();
    if (wifiCountdown == 0) {
      resetWifiManager();
    }
    delay(200);
    ESP.restart();
  }
#endif
}

// V_read / 4096 * 3.3 = V_real / ratio
// V_real = V_read / 4096 * 3.3 * ratio
// V_real = V_read / vFactor, vFactor = 4096 / 3.3 / ratio
// a more accurate fitting for V1_0 is V_real = V_read / 515 + 1.95

#ifdef VOLTAGE
bool lowBattery() {
  long currentTime = millis() / CHECK_BATTERY_PERIOD;
  if (currentTime > uptime) {
    uptime = currentTime;
    float voltage = analogRead(VOLTAGE);
#ifdef BiBoard_V1_0
    voltage = voltage / 515 + 1.9;
#else
    voltage = voltage / 414;
#endif
    if (voltage < NO_BATTERY_VOLTAGE2
        || ((voltage < LOW_VOLTAGE2                                       // powered by 6V, voltage >= NO_BATTERY && voltage < LOW_VOLTAGE2
             || (voltage > NO_BATTERY_VOLTAGE && voltage < LOW_VOLTAGE))  // powered by 7.4V
            && fabs(voltage - lastVoltage) < 0.2)                         // not caused by power fluctuation during movements
    ) {                                                                   // if battery voltage is low, it needs to be recharged
      // give the robot a break when voltage drops after sprint
      // adjust the thresholds according to your batteries' voltage
      // if set too high, the robot will stop working when the battery still has power.
      // If too low, the robot may not alarm before the battery shuts off
      lowBatteryQ = true;
      if (!safeRest) {
        // shutServos();
        // delay(2000);
        strcpy(lastCmd, "rest");
        loadBySkillName(lastCmd);
        shutServos();
        safeRest = true;
      }
      if (!batteryWarningCounter) {
        PTF("Low power: ");
        PT(voltage);
        PTLF("V. The robot won't move.");
        PTLF("Long-press the battery's button to turn it on!");
#ifdef I2C_EEPROM_ADDRESS
        if (i2c_eeprom_read_byte(EEPROM_BOOTUP_SOUND_STATE))
#else
        if (config.getBool("bootSndState", 1))
#endif
          playMelody(melodyLowBattery, sizeof(melodyLowBattery) / 2);
      }
      batteryWarningCounter = (batteryWarningCounter + 1) % BATTERY_WARNING_FREQ;
      //    strip.show();
      //       int8_t bStep = 1;
      //       for (byte brightness = 1; brightness > 0; brightness += bStep) {
      // #ifdef NEOPIXEL_PIN
      //         strip.setPixelColor(0, strip.Color(brightness, 0, 0));
      //         strip.show();
      // #endif
      // #ifdef PWM_LED_PIN
      // if (autoLedQ)
      //         analogWrite(PWM_LED_PIN, 255 - brightness);
      // #endif
      //         if (brightness == 255)
      //           bStep = -1;
      //         delay(5);
      //       }
      lastVoltage = voltage;
      return true;
    }
    if (safeRest) {
      // strcpy(lastCmd, "rest");
      // loadBySkillName(lastCmd);
      // shutServos();
      safeRest = false;
    }
    lastVoltage = voltage;
    if ((voltage > LOW_VOLTAGE + 0.2  // powered by 7.4V
         || (voltage > LOW_VOLTAGE2 + 0.2
             && voltage < NO_BATTERY_VOLTAGE))  // powered by 6V, voltage >= NO_BATTERY && voltage < LOW_VOLTAGE2
        && lowBatteryQ)                         // +0.2 to avoid fluctuation around the threshold
    {
      // if (voltage > LOW_VOLTAGE + 0.2){
      //   PT("Got ");
      //   PT(voltage);
      //   PTL(" V power");
      // }
      // else
      //   PTL("Got 6.0 V power");
      PT("Got ");
      PT(voltage);
      PTL(" V power");
      playMelody(melodyOnBattery, sizeof(melodyOnBattery) / 2);
      lowBatteryQ = false;
      batteryWarningCounter = 0;
      
      // Reactivate PWM signals to fix servo non-responsiveness after battery power restoration
      PTL("Reactivating servo PWM signals after power restoration...");
#ifdef ESP_PWM
      // Simply resend PWM signals for current positions
      for (int c = 0; c < PWM_NUM; c++) {
        servo[c].write(currentAng[c < 4 ? c : c + 4]);
      }
#else
      // Resend PCA9685 PWM signals
      for (int c = 0; c < PWM_NUM; c++) {
        pwm.writeAngle(c, currentAng[c < 4 ? c : c + 4]);
      }
#endif
      // Return to rest posture
      strcpy(newCmd, "rest");
      loadBySkillName(newCmd);
    }
  }
  return false;
}
#endif

void reaction() {  // Reminder:  reaction() is repeatedly called in the "forever" loop() of OpenCatEsp32.ino
  if (newCmdIdx) {
    // PTLF("-----");
    lowerToken = tolower(token);
    if (initialBoot) {  //-1 for marking the boot-up calibration state
      fineAdjustQ = true;
      // updateGyroQ = true;
      gyroBalanceQ = true;
      autoSwitch = RANDOM_MIND;
      initialBoot = false;
    }
#ifdef PWM_LED_PIN
    if (autoLedQ)
      digitalWrite(PWM_LED_PIN, HIGH);
#endif
    if (token != T_REST && newCmdIdx < 5)
      idleTimer = millis();
    if (newCmdIdx < 5 && lowerToken != T_BEEP && token != T_MEOW && token != T_LISTED_BIN
        && token != T_INDEXED_SIMULTANEOUS_BIN && token != T_TILT && token != T_READ && token != T_WRITE
        && token != T_JOYSTICK && token != T_EXTENSION)
      beep(15 + newCmdIdx, 5);  // ToDo: check the muted sound when newCmdIdx = -1
    if (!workingStiffness
        && (lowerToken == T_SKILL || lowerToken == T_INDEXED_SEQUENTIAL_ASC
            || lowerToken == T_INDEXED_SIMULTANEOUS_ASC)) {
#ifdef T_SERVO_MICROSECOND
      setServoP(P_WORKING);
      workingStiffness = true;
#endif
    }
    if ((lastToken == T_SERVO_CALIBRATE || lastToken == T_REST || lastToken == T_SERVO_FOLLOW || !strcmp(lastCmd, "fd"))
        && token != T_SERVO_CALIBRATE) {
      // updateGyroQ = true;
      gyroBalanceQ = true;  // This is the default state for this "Q" boolean with all tokens except (T_SERVO_CALIBRATE
                            // && when lastToken is one of the listed values)
      printToAllPorts('G');
    }
    if (token != T_PAUSE && !tStep) {
      tStep = 1;
      printToAllPorts('p');
    }
#ifdef ESP_PWM
    if (token != T_SERVO_FEEDBACK && token != T_SERVO_FOLLOW && measureServoPin != -1) {
      for (byte i = 0; i < DOF; i++)
        movedJoint[i] = 0;
      reAttachAllServos();
      measureServoPin = -1;
      readFeedbackQ = false;    // This is the default value for this "Q" boolean condition with all tokens except (those
                                // in the conditional && measureServoPin != -1)
      followFeedbackQ = false;  // This is the default state for this "Q" boolean condition with all tokens except
                                // (those in the conditional && measureServoPin != -1)
    }
#endif

    switch (token) {
      case T_HELP_INFO:
        {
          PTLF("* Please refer to docs.petoi.com.\nEnter any character to continue.");
          while (!Serial.available())
            ;
          break;
        }
      case T_QUERY:
        {
          if (cmdLen == 0) {
            printToAllPorts(MODEL);
            printToAllPorts(SoftwareVersion);
          } else {
            byte i = 0;
            while (newCmd[i] != '\0') {
              if (newCmd[i] == C_QUERY_PARTITION)
                displayNsvPartition();
              i++;
            }
          }
          break;
        }
      case T_NAME:
        {
          if (cmdLen > 16)
            printToAllPorts("ERROR! The name should be within 16 characters!");
          else if (cmdLen)
            customBleID(
              newCmd,
              cmdLen);  // customize the Bluetooth device's broadcast name. e.g. nMyDog will name the device as "MyDog"
                        // it takes effect the next time the board boosup. it won't interrupt the current connecton.
          printToAllPorts(
#ifdef I2C_EEPROM_ADDRESS
            readLongByBytes(EEPROM_DEVICE_NAME)
#else
            config.getString("ID")
#endif
          );
          break;
        }
#ifdef WEB_SERVER
    case T_WIFI_INFO:
    {
      if (!webServerConnected)
      {
        PTLF("The wifi info should start with \'w\' and followed by ssid and password, separated by %.\n e.g. w%WifiName%password");
        String wifiInfo = newCmd;
        int delimiter = wifiInfo.indexOf('%', 2); // 找到第二个%的位置
        if (delimiter != -1)
        {
          ssid = wifiInfo.substring(1, delimiter);
          password = wifiInfo.substring(delimiter + 1);
          PTHL("WifiSSID: ", ssid);
          // 只显示密码的后4位，其余用*代替
          String maskedPassword = "";
          if (password.length() > 4) {
            for (int i = 0; i < password.length() - 4; i++) {
              maskedPassword += "*";
            }
            maskedPassword += password.substring(password.length() - 4);
          } else {
            maskedPassword = password; // 如果密码长度<=4，显示全部
          }
          PTHL("Password: ", maskedPassword);
          webServerConnected = connectWifi(ssid, password);
          // 清除密码变量，防止在内存中残留
          password = "";
          if (webServerConnected)
          {
//            PTHL("Successfully connected Wifi to IP Address: ", WiFi.localIP());
            printToAllPorts("Successfully connected Wifi to IP Address: " + WiFi.localIP().toString());
#ifdef I2C_EEPROM_ADDRESS
                i2c_eeprom_write_byte(EEPROM_WIFI_MANAGER, true);
#else
                config.putBool("WifiManager", true);
#endif
            PTLF("Rebooting to use web server.");
            delay(3000);
            ESP.restart();
          }
          else
          {
            Serial.println("Timeout: Fail to connect Wifi!");
            // 连接失败时也清除密码变量
            password = "";
          }
        }
      }
      else
      {
        printToAllPorts("Wifi already connected to IP Address: " + WiFi.localIP().toString());
        PTLF("Web server should already be running");
        PTLF("Press the BOOT key to reboot and use Wifi manager.");
        PTLF("Hold the BOOT key and count down 10 if you want to clear the previous Wifi credentials.");
      }
      break;
    }
#endif
      case T_GYRO:
      case T_RANDOM_MIND:
        {
          if (token == T_RANDOM_MIND) {
            autoSwitch = !autoSwitch;
            token = autoSwitch ? 'Z' : 'z';  // G for activated gyro
          }
#ifdef GYRO_PIN
      else if (token == T_GYRO)
      {
        if (cmdLen == 0)
        {
          gyroBalanceQ = !gyroBalanceQ;
          token = gyroBalanceQ ? 'G' : 'g'; // G for activated gyro
        }
        else
        {
          if (newCmd[0] == C_GYRO_CALIBRATE)
          {
            shutServos();
            PTLF("Setting updateGyroQ to false...");
            updateGyroQ = false;
            if (newCmd[1] != C1_GYRO_CALIBRATE_IMMEDIATELY)
            {
              PTLF("\nPut the robot FLAT on the table and don't touch it during calibration.");
              beep(8, 500, 500, 5);
              beep(15, 500, 500, 1);
            }
            
            // Wait for IMU task to terminate completely
            if (TASK_imu != NULL) {
//<<<<<<< HEAD
              PTLF("Waiting for IMU task to terminate before calibration...");
              // Wait for IMU task to terminate
              while (eTaskGetState(TASK_imu) != eDeleted) {
                  if(eTaskGetState(TASK_imu)== eReady){
                      TASK_imu = NULL;
                      break;
                  }
                vTaskDelay(100 / portTICK_PERIOD_MS);
              }
//=======
//              eTaskState taskState = eTaskGetState(TASK_imu);
//              PTHL("Task state:", taskState);
//            
//              if (taskState != eDeleted) {
//                // Wait for task to terminate naturally
//                PTLF("Waiting for IMU task to terminate...");
//                unsigned long startTime = millis();
//                const unsigned long timeout = 3000; // 3 second timeout
//                
//                // Wait for timeout
//                while (TASK_imu != NULL && (millis() - startTime) <= timeout) {
//                  delay(200);
//                }
//                
//                // Handle timeout
//                if (TASK_imu != NULL) {
//                  PTLF("IMU task termination timeout - set task handle to NULL");
//                  TASK_imu = NULL;    // Set task handle to NULL
//                }
//              }
//                delay(50);  // Wait for IMU task to fully terminate
//>>>>>>> pr/35
            }
            
            // Create calibration task to run on core 0
            xTaskCreatePinnedToCore(
                taskCalibrateImuUsingCore0,         // Task function
                "taskCalibrateImuUsingCore0",       // Task name
                1800,                               // Task stack size
                NULL,                               // Task parameters
                1,                                  // Task priority
                &taskCalibrateImuUsingCore0_handle, // Task handle
                0                                   // Task core number to run on
            );
            
            // Wait for updateGyroQ to be re-set to true, indicating calibration completion
            unsigned long waitStart = millis();
            const unsigned long maxWaitTime = 10000; // Maximum waiting time 10 seconds
            
            while (!updateGyroQ) {
              if (millis() - waitStart > maxWaitTime) {
                PTLF("Timeout waiting for calibration to complete. Forcing continuation.");
                updateGyroQ = true; // Force continuation
                break;
              }
              vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            
            PTLF("Calibration completed, allowing IMU to stabilize...");
            delay(3000); // Allow IMU to stabilize after calibration
            beep(18, 50, 50, 6);

             // create IMU task
             xTaskCreatePinnedToCore(taskIMU,        // Task function
              "TaskIMU",                             // Task name
              1500,                                  // Task stack size
              &updateGyroQ,                          // Task parameters
              1,                                     // Task priority
              &TASK_imu,                             // Task handle
              0);                                    // Task core number to run on

            // Wait for task creation to complete
            delay(100);
            
            // Get task handle, for subsequent operations
            if (TASK_imu == NULL) {
              TASK_imu = xTaskGetHandle("TaskIMU");
              if (TASK_imu == NULL) {
                PTLF("Warning: Failed to get IMU task handle!");
              } else {
                PTLF("IMU task created successfully");
              }
            }
          }
          else
          {
            byte i = 0;
            while (newCmd[i] != '\0')
            {
              if (toupper(newCmd[i]) == C_GYRO_FINENESS)
              {                                               // if newCmd[i] is 'f' or 'F'
                fineAdjustQ = (newCmd[i] == C_GYRO_FINENESS); // if newCmd[i] == T_GYRO_FINENESS, fineAdjustQ is true. else newCmd[i] == C_GYRO_FINENESS_OFF, fineAdjustQ is false.
                token = fineAdjustQ ? 'G' : 'g';              // G for activated gyro
              }
              else if (toupper(newCmd[i]) == C_GYRO_BALANCE)  // if newCmd[i] is 'b' or 'B'
                gyroBalanceQ = (newCmd[i] == C_GYRO_BALANCE); // if newCmd[i] == T_GYRO_FINENESS, gyroBalanceQ is true. else is false.
              else if (toupper(newCmd[i]) == C_GYRO_UPDATE)  // if newCmd[i] is 'u' or 'U' - Update gyro reading
                updateGyroQ = (newCmd[i] == C_GYRO_UPDATE); // if newCmd[i] == 'U', updateGyroQ is true. else 'u', updateGyroQ is false.
              else if (toupper(newCmd[i]) == C_PRINT)
              {                                      // if newCmd[i] is 'p' or 'P'
                printGyroQ = (newCmd[i] == C_PRINT); // if newCmd[i] == T_GYRO_PRINT, always print gyro. else only print once
                print6Axis();
              }
              else if (newCmd[i] == '?')
              {
                PTF("Gyro state:");
                PTT(" Update-", updateGyroQ);
                PTT(" Balance-", gyroBalanceQ);
                PTT(" Print-", printGyroQ);
                PTTL(" Frequency-", fineAdjustQ);
              }
              i++;
            }
            imuSkip = fineAdjustQ ? IMU_SKIP : IMU_SKIP_MORE;
            runDelay = fineAdjustQ ? delayMid : delayShort;
          }
        }
      }
#endif
          break;
        }
      case T_PAUSE:
        {
          tStep = !tStep;             // tStep can be -1
          token = tStep ? 'p' : 'P';  // P for pause activated
          if (tStep)
            token = T_SKILL;
          else
            shutServos();
          break;
        }
#ifdef VOLTAGE
      case T_POWER:
        {
          float voltage = analogRead(VOLTAGE);
#ifdef BiBoard_V1_0
          voltage = voltage / 515 + 1.9;
#else
          voltage = voltage / 414;
#endif
          String message = "Voltage: ";
          printToAllPorts(message + voltage + " V");
          break;
        }
#endif
      case T_ACCELERATE:
        {
          runDelay = max(0, runDelay - 1);
          PTHL("Run delay", runDelay);
          break;
        }
      case T_DECELERATE:
        {
          runDelay = min(delayLong, runDelay + 1);
          PTHL("Run delay", runDelay);
          break;
        }
      case T_REST:
        {
          gyroBalanceQ = false;
          printToAllPorts('g');
          if (cmdLen == 0) {
            strcpy(newCmd, "rest");
            if (strcmp(newCmd, lastCmd)) {
              loadBySkillName(newCmd);
            }
            shutServos();
            manualHeadQ = false;
            readFeedbackQ = false;
          } else if (cmdLen == 1) {  // allow turning off a single joint
            shutServos(atoi(newCmd));
          }
          break;
        }
      case T_JOINTS:
        {  // show the list of current joint anles
          //          printRange(DOF);
          //          printList(currentAng);
          printToAllPorts('=');
          if (cmdLen)
            printToAllPorts(currentAng[atoi(newCmd)]);
          else {
            printToAllPorts(range2String(DOF));
            printToAllPorts(list2String(currentAng));
          }
          break;
        }
        // case T_MELODY:
        //   {
        //     playMelody(melody1, sizeof(melody1) / 2);
        //     break;
        //   }
#ifdef ULTRASONIC
      case T_COLOR:
        {
          if (!ultrasonicLEDinitializedQ)
            rgbUltrasonicSetup();
          if (cmdLen < 2)  // a single 'C' will turn off the manual color mode
            manualEyeColorQ = false;
          else {  // turn on the manual color mode
            manualEyeColorQ = true;
            ultrasonic.SetRgbEffect(E_RGB_INDEX(uint8_t(newCmd[3])), ultrasonic.color(newCmd[0], newCmd[1], newCmd[2]),
                                    uint8_t(newCmd[4]));
          }
          break;
        }
#endif
      case ';':
        {
          setServoP(P_SOFT);
          break;
        }
      case ':':
        {
          setServoP(P_HARD);
          break;
        }
      case T_SAVE:
        {
          PTLF("save offset");
          saveCalib(servoCalib);
#ifdef VOICE
          if (newCmdIdx == 2) {
            char setCmd[] = "Ac~";  // turn on voice
            set_voice(setCmd);
          }
#endif
          break;
        }
      case T_ABORT:
        {
          PTLF("aborted");
#ifdef I2C_EEPROM_ADDRESS
          i2c_eeprom_read_buffer(EEPROM_CALIB, (byte *)servoCalib, DOF);
#else
          config.getBytes("calib", servoCalib, DOF);
#endif
#ifdef VOICE
          if (newCmdIdx == 2) {
            char setCmd[] = "Ac~";  // turn on voice
            set_voice(setCmd);
          }
#endif
          break;
        }
      case T_RESET:
        {
          resetAsNewBoard('R');
          break;
        }
      case T_JOYSTICK:
        {
          // PTL("raw input newCmd");
          // for (int i = 0; i < cmdLen; i++)
          //   PTH(int8_t(newCmd[i]), char(newCmd[i]));
          int flush126 = 0;
          PTL();
          while ((char)newCmd[flush126] == '~')
            flush126++;
          if ((int8_t)newCmd[flush126] == -126) {  // case: button
            strcpy(buttonCmd, newCmd + flush126 + 1);
            int j = 0;
            while (buttonCmd[j] != '\0') {
              if (buttonCmd[j] == '\n' || buttonCmd[j] == '~') {
                buttonCmd[j] = '\0';
                break;
              }
              j++;
            }
            PTHL("string cmd", buttonCmd);
          } else {  // case: joystick
            int8_t currX, currY, dirX, dirY;
            int8_t center = 255 / 2 / 10, center1 = 125 / 2 / 2;
            currX = newCmd[flush126];
            currY = newCmd[flush126 + 1];
            dirX = sign(currX);  //-1,0,1
            if (abs(currX) <= center1)
              dirX = 0;
            else if (currX < -center1)
              dirX = -1;
            else if (currX > center1)
              dirX = 1;

            if (abs(currY) <= center)
              dirY = 0;
            else if (currY > center)
              dirY = 1;
            else if (currY < -center1)
              dirY = -2;
            else
              dirY = -1;
            byte dirMap[][3] = { { 11, 9, 10 }, { 8, 1, 2 }, { 7, 0, 3 }, { 6, 5, 4 } };
            char joystickDirCmd[] = { '\0', 'F', 'R', 'R', 'R', 'D', 'L', 'L', 'L', 'f', 'r', 'l' };
            PTHL(currX, currY);
            PTHL(dirX, dirY);
            char suffix[2] = { joystickDirCmd[dirMap[dirY + 2][dirX + 1]] };
            PTHL("suffix", suffix[0]);
            if (buttonCmd[0] != '\0') {
              strcat(buttonCmd, suffix);
              PTHL("joystick cmd", buttonCmd + 1);
              tQueue->addTask(buttonCmd[0], buttonCmd + 1);
              buttonCmd[0] = '\0';
              delay(500);
            }
          }
          if (buttonCmd[0] == '\0')
            break;
          break;
        }
      case T_SERVO_CALIBRATE:           // calibration
      case T_INDEXED_SEQUENTIAL_ASC:    // move multiple indexed joints to angles once at a time (ASCII format entered in
                                        // the serial monitor)
      case T_INDEXED_SIMULTANEOUS_ASC:  // move multiple indexed joints to angles simultaneously (ASCII format entered
                                        // in the serial monitor)
#ifdef T_SERVO_MICROSECOND
      case T_SERVO_MICROSECOND:  // send pulse with unit of microsecond to a servo
#endif
      case T_TILT:  // tilt the robot, format: t axis angle. 0:yaw, 1:pitch, 2:roll
      case T_MEOW:  // meow
      case T_BEEP:  // beep(tone, duration): tone 0 is pause, duration range is 0~255
#ifdef T_TUNER
      case T_TUNER:
#endif
      case T_BALANCE_SLOPE:
        {
          if (token == T_INDEXED_SIMULTANEOUS_ASC && cmdLen == 0)
            manualHeadQ = false;
          else {
            int targetFrame[DOF + 1];
            // arrayNCPY(targetFrame, currentAng, DOF);
            for (int i = 0; i < DOF; i++) {
              targetFrame[i] = currentAng[i] - (gyroBalanceQ ? currentAdjust[i] : 0);
            }
            targetFrame[DOF] = '~';
            
            char *cmdForParsing = new char[cmdLen + 1];
            strcpy(cmdForParsing, newCmd);
            if (token == T_SERVO_CALIBRATE && lastToken != T_SERVO_CALIBRATE) {
              #ifdef T_SERVO_MICROSECOND
                                setServoP(P_HARD);
                                workingStiffness = false;
              #endif
              #ifdef VOICE
                                if (newCmdIdx == 2) {  // only deactivate the voice module via serial port
              
                                  char setCmd[] = "Ad~";  // turn off voice
                                  set_voice(setCmd);
                                }
              #endif
                                strcpy(newCmd, "calib");//it will override the newCmd, so we need to backup it with originalCmd
                                loadBySkillName(newCmd);
                              }
            char *pch;
            pch = strtok(cmdForParsing, " ,");
            nonHeadJointQ = false;
            do {  // it supports combining multiple commands at one time
              // for example: "m8 40 m8 -35 m 0 50" can be written as "m8 40 8 -35 0 50"
              // the combined commands should be less than four. string len <=30 to be exact.
              int target[2] = {};
              int inLen = 0;
              for (byte b = 0; b < 2 && pch != NULL; b++) {
                target[b] = atoi(pch);  //@@@ cast
                pch = strtok(NULL, " ,\t");
                inLen++;
              }
              // PTHL( target[0],target[1]);
              if ((token == T_INDEXED_SEQUENTIAL_ASC || token == T_INDEXED_SIMULTANEOUS_ASC) && target[0] >= 0
                  && target[0] < DOF) {
                targetFrame[target[0]] = target[1];
                if (target[0] < 4) {
                  targetHead[target[0]] = target[1];
                  manualHeadQ = true;
                } else
                  nonHeadJointQ = true;
              }
              if (token == T_SERVO_CALIBRATE) {
                gyroBalanceQ = false;
                if (target[0] == DOF) {  // auto calibrate all body joints using servos' angle feedback
                  strcpy(newCmd, "rest");
                  loadBySkillName(newCmd);
                  shutServos();
                  printToAllPorts(
                    T_SERVO_CALIBRATE);  // avoid the confirmation token blocked by the loop in autoCalibrate function.
                  autoCalibrate();
                  break;
                }
                if (inLen == 2) {
                  if (target[1] >= 1001) {  // Using 1001 for incremental calibration. 1001 is adding 1 degree, 1002 is
                                            // adding 2 and 1009 is adding 9 degrees
                    target[1] = servoCalib[target[0]] + target[1] - 1000;
                  } else if (target[1] <= -1001) {  // Using -1001 for incremental calibration. -1001 is removing 1
                                                    // degree, 1002 is removing 2 and 1009 is removing 9 degrees
                    target[1] = servoCalib[target[0]] + target[1] + 1000;
                  }
                  servoCalib[target[0]] = target[1];
                }
#if defined ROBOT_ARM && defined GYRO_PIN
                if (target[0] == -2)  // auto calibrate the robot arm's pincer
                {
                  // loadBySkillName("triStand");
                  // shutServos();
                  servoCalib[2] = -30;
                  calibratedPWM(2, 0);
                  calibratedPWM(1, 90);
                  delay(500);
                  int criticalAngle = calibratePincerByVibration(-25, 25, 4);
                  criticalAngle = calibratePincerByVibration(criticalAngle - 4, criticalAngle + 4, 1);
                  servoCalib[2] = servoCalib[2] + criticalAngle + 16;
                  PTHL("Pincer calibrate angle: ", servoCalib[2]);
#ifdef I2C_EEPROM_ADDRESS
                  i2c_eeprom_write_byte(EEPROM_CALIB + 2, servoCalib[2]);
#else
                  config.putBytes("calib", servoCalib, DOF);
#endif
                  calibratedZeroPosition[2] = zeroPosition[2] + float(servoCalib[2]) * rotationDirection[2];
                  loadBySkillName("calib");
                } else
#endif
                  if (target[0] < DOF && target[0] >= 0) {
                  int duty = zeroPosition[target[0]] + float(servoCalib[target[0]]) * rotationDirection[target[0]];
                  if (PWM_NUM == 12 && WALKING_DOF == 8 && target[0] > 3
                      && target[0] < 8)  // there's no such joint in this configuration
                    continue;
                  int actualServoIndex = (PWM_NUM == 12 && target[0] > 3) ? target[0] - 4 : target[0];
#ifdef ESP_PWM
                  servo[actualServoIndex].write(duty);
#else
                  pwm.writeAngle(actualServoIndex, duty);
#endif
                }
              } else if (token == T_INDEXED_SEQUENTIAL_ASC) {
                transform(targetFrame, 1, 1);
                delay(10);
              }
#ifdef T_SERVO_MICROSECOND
              else if (token == T_SERVO_MICROSECOND) {  // there might be some problems.
#ifdef ESP_PWM
                servo[PWM_pin[target[0]]].writeMicroseconds(target[1]);
#else
                pwm.writeMicroseconds(PWM_pin[target[0]], target[1]);
#endif
              }
#endif
#ifdef GYRO_PIN
              else if (token == T_TILT) {
                yprTilt[target[0]] = target[1];
              }
#endif
              else if (token == T_MEOW) {
                meow(random() % 2 + 1, (random() % 4 + 2) * 10);
              } else if (token == T_BEEP) {
                if (inLen == 0) {  // toggle on/off the bootup melody

#ifdef I2C_EEPROM_ADDRESS
                  soundState = !i2c_eeprom_read_byte(EEPROM_BOOTUP_SOUND_STATE);
                  i2c_eeprom_write_byte(EEPROM_BOOTUP_SOUND_STATE, soundState);
#else
                  soundState = !config.getBool("bootSndState");
                  config.putBool("bootSndState", soundState);
#endif
                  printToAllPorts(soundState ? "Unmute" : "Muted");
                  if (soundState && !buzzerVolume) {  // if i want to unmute but the volume was set to 0
                    buzzerVolume = 5;                 // set the volume to 5/10
#ifdef I2C_EEPROM_ADDRESS
                    i2c_eeprom_write_byte(EEPROM_BUZZER_VOLUME, buzzerVolume);
#else
                    config.putChar("buzzerVolume", buzzerVolume);
#endif
                    playMelody(volumeTest, sizeof(volumeTest) / 2);
                  }
                } else if (inLen == 1) {                      // change the buzzer's volume
                  buzzerVolume = max(0, min(10, target[0]));  // in scale of 0~10
                  if (soundState ^ (buzzerVolume > 0))
                    printToAllPorts(buzzerVolume ? "Unmute" : "Muted");  // only print if the soundState changes
                  soundState = buzzerVolume;
#ifdef I2C_EEPROM_ADDRESS
                  i2c_eeprom_write_byte(EEPROM_BOOTUP_SOUND_STATE, soundState);
                  i2c_eeprom_write_byte(EEPROM_BUZZER_VOLUME, buzzerVolume);
#else
                  config.putBool("bootSndState", soundState);
                  config.putChar("buzzerVolume", buzzerVolume);
#endif
                  PTF("Changing volume to ");
                  PT(buzzerVolume);
                  PTL("/10");
                  playMelody(volumeTest, sizeof(volumeTest) / 2);
                } else if (target[1] > 0) {
                  beep(target[0], 1000 / target[1]);
                }
              }
#ifdef T_TUNER
              else if (token == T_TUNER) {
                if (inLen> 1) {
                  *par[target[0]] = target[1];
                  PT(target[0]);
                  PT('\t');
                  PTL(target[1]);
                }
              }
#endif
              else if (token == T_BALANCE_SLOPE) {
                if (inLen == 2) {
                  balanceSlope[0] = max(-2, min(2, target[0]));
                  balanceSlope[1] = max(-2, min(2, target[1]));
                }
              }
              // delay(5);
            } while (pch != NULL);
            
            // Release dynamically allocated memory
            delete[] cmdForParsing;
            
            // For calibration commands, print calibration values after the loop
            if (token == T_SERVO_CALIBRATE) {
              printToAllPorts(range2String(DOF));
              printToAllPorts(list2String(servoCalib));
            }
          
#ifdef T_TUNER
            if (token == T_TUNER) {
              for (byte p = 0; p < sizePars; p++) {
                PT(*par[p]);
                PT('\t');
              }
              PTL();
            }
#endif
            if ((token == T_INDEXED_SEQUENTIAL_ASC || token == T_INDEXED_SIMULTANEOUS_ASC)
                && (nonHeadJointQ || lastToken != T_SKILL)) {
              // printToAllPorts(token);
              transform(targetFrame, 1, transformSpeed);  // if (token == T_INDEXED_SEQUENTIAL_ASC) it will be useless
              skill->convertTargetToPosture(targetFrame);
            }
            // if (token == T_INDEXED_SEQUENTIAL_ASC)
            //   skill->convertTargetToPosture();
            // if (token == T_INDEXED_SIMULTANEOUS_ASC) {
            //   PTL(token);  //make real-time motion instructions more timely
            //   if (nonHeadJointQ || lastToken != T_SKILL) {
            //     transform(targetFrame, 1, 4);
            //     skill->convertTargetToPosture();
            //   }
            // }
            delete[] pch;
          }
          break;
        }

      // this block handles array like arguments
      case T_INDEXED_SEQUENTIAL_BIN:
      case T_INDEXED_SIMULTANEOUS_BIN:
      case T_READ:
      case T_WRITE:
        {  // indexed joint motions: joint0, angle0, joint1, angle1, ... (binary encoding)
          if (cmdLen < 2)
            manualHeadQ = false;
          else {
            int targetFrame[DOF + 1];
            for (int i = 0; i < DOF; i++) {
              targetFrame[i] = currentAng[i] - (gyroBalanceQ ? currentAdjust[i] : 0);
            }
            targetFrame[DOF] = '~';
            byte group = token == T_WRITE ? 3 : 2;
            for (int i = 0; i < cmdLen; i += group) {
              int8_t cmdIndex = (int8_t)newCmd[i];  // Convert to int8_t to avoid char subscript warnings
              if (cmdIndex >= 0 && cmdIndex < DOF) {
                targetFrame[cmdIndex] = (int8_t)newCmd[i + 1];
                if (cmdIndex < 4) {
                  targetHead[cmdIndex] = (int8_t)newCmd[i + 1];
                  manualHeadQ = true;
                } else
                  nonHeadJointQ = true;
              }
              if (token == T_INDEXED_SEQUENTIAL_BIN) {
                transform(targetFrame, 1, transformSpeed);
                delay(10);
              } else if (token == T_WRITE) {  // Write a/d pin value
                pinMode((uint8_t)newCmd[i + 1], OUTPUT);
                if (newCmd[i] == TYPE_ANALOG) {
                  analogWrite(
                    (uint8_t)newCmd[i + 1],
                    uint8_t(newCmd[i + 2]));  // analog value can go up to 255.
                                              // the value was packed as unsigned byte by ardSerial
                                              // but casted by readSerial() as signed char and saved into newCmd.
                } else if (newCmd[i] == TYPE_DIGITAL)
                  digitalWrite((uint8_t)newCmd[i + 1], (uint8_t)newCmd[i + 2]);
              } else if (token == T_READ) {  // Read a/d pin
                // 34 35 36 37 38 39 97 100
                // "  #  $  %  &  '  a  d
                // e.g. analogRead(35) = Ra# in the Serial Monitor
                //                     = [R,a,35] in the Python API
                printToAllPorts('=');
                pinMode((uint8_t)newCmd[i + 1], INPUT);
                if (newCmd[i] == TYPE_ANALOG)  // Arduino Uno: A2->16, A3->17
                  printToAllPorts(analogRead((uint8_t)newCmd[i + 1]));
                else if (newCmd[i] == TYPE_DIGITAL)
                  printToAllPorts(digitalRead((uint8_t)newCmd[i + 1]));
              }
            }
            if (nonHeadJointQ || lastToken != T_SKILL) {
              // printToAllPorts(token);
              transform(targetFrame, 1, transformSpeed);  // if (token == T_INDEXED_SEQUENTIAL_BIN) it will be useless
              skill->convertTargetToPosture(targetFrame);
            }
            // if (token == T_INDEXED_SEQUENTIAL_BIN)
            //   skill->convertTargetToPosture();
            // if (token == T_INDEXED_SIMULTANEOUS_BIN) {
            //   PTL(token);  //make real-time motion instructions more timely
            //                // if (lastToken != T_SKILL)
            //   if (nonHeadJointQ || lastToken != T_SKILL) {
            //     transform(targetFrame, 1, 4);
            //     skill->convertTargetToPosture();
            //   }
            // }
          }
          break;
        }
      case T_EXTENSION:
        {
          // PTH("cmdLen = ", cmdLen);
          if (newCmd[0] == '?')
            showModuleStatus();
          else if (newCmd[0] != 'U' || (newCmd[0] == 'U' && cmdLen == 1)) {  // when reading the distance from ultrasonic sensor, the cmdLen is 3.
            // and we don't want to change the activation status of the ultrasonic sensor behavior
            reconfigureTheActiveModule(newCmd);
          }

          // deal with the following command
          switch (newCmd[0]) {
#ifdef VOICE
            case EXTENSION_VOICE:
              {
                set_voice(newCmd);
                break;
              }
#endif
#ifdef ULTRASONIC
      case EXTENSION_ULTRASONIC:
      {
        if (cmdLen >= 3)
        {
          int distance = readUltrasonic((int8_t)newCmd[1], (int8_t)newCmd[2]);
          printToAllPorts('=');
          printToAllPorts(distance);
#ifdef WEB_SERVER
          sendUltrasonicData(distance); // 发送超声波数据到WebSocket客户端
#endif
        }
        break;
      }
#endif
#ifdef CAMERA
            case EXTENSION_CAMERA:
              {
                char *option = newCmd;
                while (*(++option) != '~') {
                  if (*option == 'P')
                    cameraPrintQ = 2;
                  else if (*option == 'p')
                    cameraPrintQ = 1;
                  else if (*option == 'R')
                    cameraReactionQ = true;
                  else if (*option == 'r')
                    cameraReactionQ = false;
                }

                if (cameraPrintQ == 1 && cameraTaskActiveQ) {
                  printToAllPorts('=');
                  showRecognitionResult(xCoord, yCoord, width, height);
                  //PTL();
                  // printToAllPorts(token);
                  cameraPrintQ = 0;  // if the command is XCp, the camera will print the result only once
                }

                break;
              }
#endif
#ifdef GESTURE
            case EXTENSION_GESTURE:
              {
                char *option = newCmd;
                while (*(++option) != '~') {
                  if (*option == 'P')
                    gesturePrintQ = 2;
                  else if (*option == 'p')
                    gesturePrintQ = 1;
                  else if (*option == 'R')
                    gestureReactionQ = true;
                  else if (*option == 'r')
                    gestureReactionQ = false;
                }

                if (gesturePrintQ == 1) {
                  int readGesture = read_gesture();
                  printToAllPorts('=');
                  if (readGesture != GESTURE_NONE)
                    printToAllPorts(readGesture);
                  gesturePrintQ = 0;  // if the command is XGp, the gesture will print the detected result only once
                }
                break;
              }
#endif
          }
          break;
        }
      case T_LISTED_BIN:  // list of all 16 joint: angle0, angle2,... angle15 (binary encoding)
        {
          transform((int8_t *)newCmd, 1, transformSpeed);  // need to add angleDataRatio if the angles are large
          break;
        }
      case T_BEEP_BIN:
        {
          if (cmdLen == 0) {  // toggle on/off the bootup melody
#ifdef I2C_EEPROM_ADDRESS
            soundState = !i2c_eeprom_read_byte(EEPROM_BOOTUP_SOUND_STATE);
            i2c_eeprom_write_byte(EEPROM_BOOTUP_SOUND_STATE, soundState);
#else
            soundState = !config.getBool("bootSndState");
            config.putBool("bootSndState", soundState);
#endif
            printToAllPorts(soundState ? "Unmute" : "Muted");
          } else {
            for (byte b = 0; b < cmdLen / 2; b++) {
              if ((int8_t)newCmd[2 * b + 1] > 0)
                beep((int8_t)newCmd[2 * b], 1000 / (int8_t)newCmd[2 * b + 1]);
            }
          }
          break;
        }
      case T_CPG:
        {
          // a: amplitude
          // t: loopStep
          // d: delay
          // p: phase
          // s: shift
// {20,-8,4,3,1,2,35,1,35,1},//small
// {36,-8,4,3,1,2,35,1,35,1},//large
// {16,-6,-4,3,1,3,21,51,31,1},//walk
// {20,-14,4,3,1,2,70,70,1,1},//bound
// {23,-10,8,3,3,1,70,70,1,1},//bound2
// {17,0,0,3,1,3,1,75,50,25},//heng
// {15,-4,-4,3,1,3,36,52,52,36},//heng2
// {18,0,0,3,1,2,35,46,35,46}//turnR
          updateCPG();
          break;
        }
      case T_CPG_BIN:
        {
          // Binary CPG parameters: 12 int8_t values + '~' terminator
          // Apply the parameters directly to CPG
          amplitude = (int8_t)newCmd[0];
          sideRatio = (int8_t)newCmd[1];
          stateSwitchAngle = (int8_t)newCmd[2];
          shift[0] = (int8_t)newCmd[3];
          shift[1] = (int8_t)newCmd[4];
          loopDelay = (int8_t)newCmd[5];
          skipStep[0] = (int8_t)newCmd[6];
          skipStep[1] = (int8_t)newCmd[7];
          phase[0] = (int8_t)newCmd[8];
          phase[1] = (int8_t)newCmd[9];
          phase[2] = (int8_t)newCmd[10];
          phase[3] = (int8_t)newCmd[11];
          
          if (cpg == NULL)
            cpg = new CPG(300, skipStep);
          else if (skipStep[0] != cpg->_skipStep[0] || skipStep[1] != cpg->_skipStep[1]) {
            delete cpg;
            cpg = new CPG(300, skipStep);
          }
          cpg->setPar(amplitude, sideRatio, stateSwitchAngle, loopDelay, shift, phase);
          cpg->printCPG();
          gyroBalanceQ = false;
          break;
        }
      case T_SIGNAL_GEN:  // resolution, speed, jointIdx, midpoint, amp, freq,phase
        {
          char *pch = strtok(newCmd, " ,");
          int inLen = 0;
          int8_t pars[60];  // allows 12 joints 5*12 = 60
          while (pch != NULL) {
            pars[inLen++] = atoi(pch);  //@@@ cast
            pch = strtok(NULL, " ,\t");
          }
          // for (int i = 0; i < inLen; i++)
          //   PTT(pars[i], ' ');
          // PTL();
          int8_t resolution = pars[0];
          int8_t speed = pars[1];
          signalGenerator(resolution, speed, pars + 2, inLen, 1);
          break;
        }
#ifdef T_SERVO_FEEDBACK
      case T_SERVO_FOLLOW:
        {
          followFeedbackQ =
            true;  // Set the condition value to true for access to servoFollow() at the end of reaction().
          newCmd[0] = C_FOLLOW;
          // no break here to keep compatible with previous 'F' function
        }
        [[fallthrough]];      // Explicitly indicate intentional fall-through
      case T_SERVO_FEEDBACK:  // f: print all angles
                              // fIndex: print the servo angle at index
                              // fp: print all angles once
                              // fP: print all angles continuously
                              // fl: learn skill by manually dragging the joints
                              // fr: replay the learned skill
        //
        {
          if (!readFeedbackQ) {
            setServoP(P_SOFT);
            workingStiffness = false;
            gyroBalanceQ = false;
            readFeedbackQ =
              true;  // Set the condition value to true for access to servoFeedback() at the end of reaction()
            // measureServoPin = (inLen == 1) ? target[0] : 16;
            if (newCmd[0] == '\0')
              measureServoPin = 16;
          }
          if (isDigit(newCmd[0])) {  // the index of servo to read feedback
            followFeedbackQ = false;
            int num = atoi(newCmd);
            if (num > 2500 && num < 4000) {
              feedbackSignal = num;
              PTF("Change feedback signal to ");
              PTL(feedbackSignal);
            } else {
              measureServoPin = num;
              PTHL("read pin", num);
            }
          } else if (cmdLen > 0) {
            if (toupper(newCmd[0]) == C_PRINT) {
              readFeedbackQ = (newCmd[0] == C_PRINT);  // If the Character command is to "continuously print", set to
                                                       // true.  Otherwise set to false for "print only once".
              servoFeedback(measureServoPin);          // Always print the servo angles at least once
            } else if (toupper(newCmd[0]) == C_FOLLOW) {
              followFeedbackQ = (newCmd[0] == C_FOLLOW);
              if (followFeedbackQ) {
                setServoP(P_SOFT);
                shutServos();
                delay(100);
                workingStiffness = false;
                gyroBalanceQ = false;
                measureServoPin = 16;
              }
            } else if (newCmd[0] == C_LEARN) {
              bool gyroLag = gyroBalanceQ;
              gyroBalanceQ = false;
              loadBySkillName("up");
              delay(500);
              shutServos();
              delay(100);
              learnByDrag();
              gyroBalanceQ = gyroLag;
            } else if (newCmd[0] == C_REPLAY) {  // perform
              loadBySkillName("up");
              performLearn();
              loadBySkillName("up");
              shutServos(0);
              readFeedbackQ = false;
            }
          }
          break;
        }
#endif
      case T_TEMP:
        {  // call the last skill data received from the serial port
#ifdef I2C_EEPROM_ADDRESS
          // Use uint16 version to prevent address corruption when reading values > 32767
          // Ensures proper address range 0-65535 without negative value conversion issues
          loadDataFromI2cEeprom((unsigned int)i2c_eeprom_read_uint16(SERIAL_BUFF));
#else
          config.getBytes("tmp", newCmd, config.getBytesLength("tmp"));
#endif
          skill->buildSkill();
          skill->transformToSkill(skill->nearestFrame());
          printToAllPorts(token);
          token = T_SKILL;
          strcpy(newCmd, "tmp");
          break;
        }
      case T_SKILL_DATA:  // takes in the skill array from the serial port, load it as a regular skill object and run it
                          // locally without continuous communication with the master
        {
#ifdef I2C_EEPROM_ADDRESS
          unsigned int i2cEepromAddress = SERIAL_BUFF + 2;  // + esp_random() % (EEPROM_SIZE - SERIAL_BUFF - 2 - 2550);
                                                            // //save to random position to protect the EEPROM
          // Use uint16 version to properly handle addresses > 32767 without sign extension issues
          i2c_eeprom_write_uint16(SERIAL_BUFF, (uint16_t)i2cEepromAddress);  // the address takes 2 bytes to store
          copydataFromBufferToI2cEeprom(i2cEepromAddress, (int8_t *)newCmd);
#else
          int bufferLen = dataLen(newCmd[0]);
          config.putBytes("tmp", newCmd, bufferLen);
#endif
          skill->buildSkill();
          skill->transformToSkill(skill->nearestFrame());
          manualHeadQ = false;
          strcpy(newCmd, "tmp");
          break;
        }
      case T_SKILL:
        {
          // Parse skill command and arguments
          char skillName[CMD_LEN + 1];
          char *spacePos = strchr(newCmd, ' ');
          int timeOrAngle = 0;
          
          if (spacePos != NULL) {
            // Extract skill name (everything before the space)
            int skillNameLen = spacePos - newCmd;
            strncpy(skillName, newCmd, skillNameLen);
            skillName[skillNameLen] = '\0';
            
            // Extract argument (everything after the space)
            timeOrAngle = atoi(spacePos + 1);
          } else {
            // No arguments, use the full command as skill name
            strcpy(skillName, newCmd);
          }
          
          if (!strcmp("x", skillName)        // x for random skill
              || strcmp(lastCmd, skillName)  // won't transform for the same gait.
              || skill->period <= 1) {    // skill->period can be NULL!
            // it's better to compare skill->skillName and newCmd.
            // but need more logics for non skill cmd in between
            if (!strcmp(skillName, "bk"))
              strcpy(skillName, "bkF");
            
            loadBySkillName(skillName);  // skillName will be overwritten as dutyAngles then recovered from skill->skillName
            manualHeadQ = false;
            
            // Handle gait control with arguments
            if (skill->period > 1 && timeOrAngle > 0) {
              char lastChar = skillName[strlen(skillName) - 1];
              
              if (lastChar == 'F') {
                // Straight gait - handle cycle-based or time-based control
                if (timeOrAngle < 200) {
                  // Cycle-based control: count gait cycles directly
                  cycleCountingMode = true;
                  targetCycles = timeOrAngle;
                  completedCycles = 0;
                  PTH("Cycle counting mode: ", targetCycles);
                  PTHL(" cycles, Period: ", skill->period);
                } else {
                  // Time-based control: use task queue with timing
                  cycleCountingMode = false;
                  char taskCmd[CMD_LEN + 1];
                  sprintf(taskCmd, "%s", skillName);
                  tQueue->addTask('k', taskCmd, timeOrAngle);
                  tQueue->addTask('k', "up");
                  PTH("Time-based mode: ", taskCmd);
                  PTHL(" for ", timeOrAngle);
                  PTL(" ms");
                }
              } else if (lastChar == 'L' || lastChar == 'R') {
                // Turning gait - set up turning control
                // Note: wkR should turn counterclockwise (negative yaw), wkL should turn clockwise (positive yaw)
                // This matches polar coordinate convention where positive angle is counterclockwise
                turningQ = true;
                initialYawAngle = ypr[0];  // ypr[0] is already negated to match polar coordinate convention
                targetYawAngle = initialYawAngle + (lastChar == 'R' ? -timeOrAngle : timeOrAngle);
                
                // Normalize target angle to -180 to 180 range
                while (targetYawAngle > 180) targetYawAngle -= 360;
                while (targetYawAngle < -180) targetYawAngle += 360;
                
                PTH("Started turning gait: ", skillName);
                PTHL(" initial yaw: ", initialYawAngle);
                PTHL(" target angle: ", targetYawAngle);
                PTHL(" turning direction: ", lastChar == 'R' ? "RIGHT (CCW)" : "LEFT (CW)");
              }
            }
            // if (skill->period > 0)
            //   printToAllPorts(token);
            // skill->info();
          }
          break;
        }
      case T_TASK_QUEUE:
        {
          tQueue->createTask();  // use 'q' to start the sequence.
                                 // add subToken followed by the subCommand
                                 // use ':' to add the delay time (mandatory)
                                 // add '>' to end the sub command
                                 // example: qk sit:1000>m 8 0 8 -30 8 0:500>
                                 // Nybble wash face: qksit:100>o 1 0, 0 40 -20 4 0, 1 -30 20 4 30, 8 -70 10 4 60, 12 -10
                                 // 10 4 0, 15 10 0 4 0:100>
          break;
        }
      default:
        {
          printToAllPorts("Undefined token!");
          printToAllPorts(newCmd);
          break;
        }
    }

    if (token == T_SKILL && newCmd[0] != '\0') {
      // if (skill->period > 0)
      strcpy(lastCmd, newCmd);
      // else
      //   strcpy(lastCmd, "up");
    }

    if (token != T_SKILL || skill->period > 0) {  // it will change the token and affect strcpy(lastCmd, newCmd)
      printToAllPorts(token);                     // postures, gaits and other tokens can confirm completion by sending the token back
      if (lastToken == T_SKILL
          && (lowerToken == T_GYRO || lowerToken == T_INDEXED_SIMULTANEOUS_ASC || lowerToken == T_INDEXED_SEQUENTIAL_ASC
              || lowerToken == T_PAUSE || token == T_JOINTS || token == T_RANDOM_MIND || token == T_BALANCE_SLOPE
              || token == T_ACCELERATE || token == T_DECELERATE || token == T_TILT))
        token = T_SKILL;
    }
#ifdef WEB_SERVER
    finishWebCommand();
#endif
    resetCmd();
#ifdef PWM_LED_PIN
    if (autoLedQ)
      digitalWrite(PWM_LED_PIN, LOW);
#endif
  }
  if (tolower(token) == T_SKILL) {
#ifdef PWM_LED_PIN
    if (autoLedQ)
      analogWrite(PWM_LED_PIN, abs(currentAng[8]));
#endif
    skill->perform();
    if (skill->period > 1) {
      delay(delayShort
            + max(0, int(runDelay
#ifdef GYRO_PIN
                         - gyroBalanceQ * (max(fabs(ypr[1]) / 2, fabs(ypr[2])) / 20)  // accelerate gait when tilted
                             / (!fineAdjustQ && !mpuQ ? 4 : 1)                        // reduce the adjust if not using mpu6050
#endif
                         )));
    }
    if (skill->period < 0) {
      if (!strcmp(skill->skillName, "fd")) {  // need to optimize logic to combine "rest" and "fold"
        shutServos();
        gyroBalanceQ = false;
        printToAllPorts('g');
        idleTimer = 0;
        token = '\0';
      } else {
        // newCmd[0] = '\0';
        // arrayNCPY(skill->dutyAngles, skill->dutyAngles + (abs(skill->period) - 1) * skill->frameSize, DOF);
        // skill->period = 1;
        // frame = 0;
        if (interruptedDuringBehavior) {
          loadBySkillName("up");
        } else
          skill->convertTargetToPosture(currentAng);
      }
      for (int i = 0; i < DOF; i++)
        currentAdjust[i] = 0;
      printToAllPorts(token);  // behavior can confirm completion by sending the token back
#ifdef GYRO_PIN
      if (xyzReal[2] > 0 && (fabs(ypr[1]) > 45 || fabs(ypr[2]) > 45)) {  // wait for imu to update
        while (fabs(ypr[1]) > 10 || fabs(ypr[2]) > 10) {
          // print6Axis();
          delay(IMU_PERIOD);
        }
      }
#endif
    }
    // if (imuException && lastCmd[strlen(lastCmd) - 1] < 'L' && skillList->lookUp(lastCmd) > 0) {  //can be simplified
    // here.
    //   if (lastCmd[0] != '\0')
    //     loadBySkillName(lastCmd);

    // if (tQueue->cleared() && tQueue->lastTask != NULL) {
    //   PT("Use last task ");
    //   tQueue->loadTaskInfo(tQueue->lastTask);
    //   delete tQueue->lastTask;
    //   tQueue->lastTask = NULL;
    //   PTL(newCmd);
    // }
  }

  // The code from here to the end of reaction() will conditionally run every time loop() in OpenCatEsp32.ino runs

  else if (followFeedbackQ) {  // Conditionally follow servo feedback to gather servo angles
    if (servoFollow()) {       // don't move the joints if no manual movement is detected
      reAttachAllServos();
      setServoP(P_SOFT);
      workingStiffness = false;
      transform((int8_t *)newCmd, 1, 2);
    }
  } else if (token == T_CPG || token == T_CPG_BIN) {
    if (cpg != NULL)
      cpg->sendSignal();
  } else if (readFeedbackQ)  // Conditionally read servo feedback and print servo angles
    servoFeedback(measureServoPin);
  // }
  else
#ifdef GESTURE
    if (gesturePrintQ == 2) {
    if (gestureGetValue != GESTURE_NONE)
      printToAllPorts(gestureGetValue);
  }
#endif
#ifdef CAMERA
  if (cameraPrintQ == 2 && cameraTaskActiveQ) {
    if (xCoord != lastXcoord || yCoord != lastYcoord)
    {
      showRecognitionResult(xCoord, yCoord, width, height);
#ifdef WEB_SERVER
      sendCameraData(xCoord, yCoord, width, height); // Send camera data to WebSocket client
#endif
      lastXcoord = xCoord;
      lastYcoord = yCoord;
    }
  } else if (!cameraTaskActiveQ)
#endif
  {
    delay(1);  // avoid triggering WDT
  }
}

// Async web processing functions
#ifdef WEB_SERVER
void finishWebCommand() {
  if (cmdFromWeb) {
    completeWebTask();  // call async completion function
    // cmdFromWeb will be set to false in completeWebTask()
  }
}
#endif
