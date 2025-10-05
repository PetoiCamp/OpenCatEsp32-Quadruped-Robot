#define MEMORY_ADDRESS_SIZE 4
class SkillPreview {
public:
  char *skillName;  // use char array instead of String to save memory
  int period;       // the period of a skill. 1 for posture, >1 for gait, <-1 for behavior
  int index;
  SkillPreview(int s) {
    skillName = new char[strlen(skillNameWithType[s])];
    strcpy(skillName, skillNameWithType[s]);
    skillName[strlen(skillNameWithType[s]) - 1] = '\0';  // drop the last charactor of skill type
    unsigned int pgmAddress = (unsigned int)progmemPointer[s];
    period = (int8_t)pgm_read_byte(pgmAddress);  // automatically cast to char*
    index = s;
  }
};

class SkillList : public QList<SkillPreview *> {
public:
  SkillList() {
    PT("Build skill list...");
    //  PT(sizeof(progmemPointer) / MEMORY_ADDRESS_SIZE);
    PTL(sizeof(skillNameWithType) / MEMORY_ADDRESS_SIZE);
    for (int s = 0; s < sizeof(progmemPointer) / MEMORY_ADDRESS_SIZE; s++) {
      SkillPreview *tempAddress = new SkillPreview(s);
      this->push_back(tempAddress);
    }
    //  PTF("free memory: ");//before building the skill list
    //  PTL(ESP.getFreeHeap());
    for (randomMindListLength = 0; randomMindList[randomMindListLength] != NULL; randomMindListLength++)
      ;
  }
  int lookUp(const char *key) {
    byte nSkills = sizeof(progmemPointer) / MEMORY_ADDRESS_SIZE;
    byte randSkillIdx = strcmp(key, "x") ? nSkills : random(nSkills);
    byte keyLen = strlen(key);
    char lr = key[keyLen - 1];
    for (int s = 0; s < nSkills; s++) {
      char readName[CMD_LEN + 1];
      strcpy(readName, this->get(s)->skillName);
      char readNameLR = readName[strlen(readName) - 1];
      if (s == randSkillIdx) {
        bool forbiddenQ = false;
        for (int i = 0; i < sizeof(forbiddenSkills) / sizeof(String); i++) {
          if (forbiddenSkills[i] == readName) {
            forbiddenQ = true;
            break;
          }
        }
        if (readNameLR == 'L' || readNameLR == 'F' || forbiddenQ) {  // forbid walking or violent motions in random mode
          randSkillIdx++;
          continue;
        }
      }
      byte nameLen = strlen(readName);
      if (s == randSkillIdx          // random skill
          || !strcmp(readName, key)  // exact match: gait type + F or L, behavior
          // || readName[nameLen - 1] == 'L' && !strncmp(readName, key, nameLen - 1)
          || (readName[nameLen - 1] != 'F' && strcmp(readName, "bk") && !strncmp(readName, key, keyLen - 1) && (lr == 'L' || lr == 'R' || lr == 'X'))  // L, R or X
      ) {
        printToAllPorts(readName);
        return s;
      }
    }
    PT('?');  // key not found
    PT(key);
    PTL('?');  // it will print ?? in random mode. Why?
    return -1;
  }
};
SkillList *skillList;

class Skill {
public:
  char skillName[20];  // use char array instead of String to save memory
  int8_t offsetLR;
  int period;  // the period of a skill. 1 for posture, >1 for gait, <-1 for behavior
  float transformSpeed;
  byte skillHeader;
  byte frameSize;
  int expectedRollPitch[2];  // expected body orientation (roll, pitch)
  byte angleDataRatio;       // divide large angles by 1 or 2. if the max angle of a skill is >128, all the angls will be divided by 2
  int8_t loopCycle[3];       // the looping section of a behavior (starting row, ending row, repeating cycles)
  byte firstMotionJoint;
  int8_t *dutyAngles;  // the data array for skill angles and parameters

  Skill() {
    skillName[0] = '\0';  // use char array instead of String to save memory
    offsetLR = 0;
    period = 0;
    transformSpeed = 1;
    frameSize = 0;
    expectedRollPitch[0] = expectedRollPitch[1] = 0;
    angleDataRatio = 1;
    loopCycle[0] = loopCycle[1] = loopCycle[2] = 0;
    firstMotionJoint = 0;
    dutyAngles = NULL;
  }
  void buildSkill() {  // K token
    strcpy(skillName, "tmp");
    offsetLR = 0;
    period = (int8_t)newCmd[0];  // automatically cast to char*
    dataLen(period);
    formatSkill();
  }

  void buildSkill(int s) {
    strcpy(skillName, newCmd);
    unsigned int pgmAddress = (unsigned int)progmemPointer[s];
    period = (int8_t)pgm_read_byte(pgmAddress);  // automatically cast to char*
    for (int i = 0; i < dataLen(period); i++) {
      newCmd[i] = pgm_read_byte(pgmAddress++);
    }
    newCmd[dataLen(period)] = '~';
    formatSkill();
  }
  ~Skill() {
  }
  int dataLen(int8_t p) {
    skillHeader = p > 0 ? 4 : 7;
    frameSize = p > 1 ? WALKING_DOF :  // gait
                  p == 1 ? DOF
                         :  // posture
                  DOF + 4;  // behavior
    int len = skillHeader + abs(p) * frameSize;
    return len;
  }

  void inplaceShift() {
    int angleLen = abs(period) * frameSize;  // need one extra byte for terminator '~'
    // int shiftRequiredByNewCmd = CMD_LEN - skillHeader + 1;  // required shift to store CMD_LEN + 1 chars. it can hold a command with CMD_LEN chars. the additioanl byte is required by '\0'.
    spaceAfterStoringData = BUFF_LEN - angleLen - 1;  // the bytes before the dutyAngles. The allowed command's bytes needs to -1
    // PTH("request", shiftRequiredByNewCmd);
    // PTH("aloShft", BUFF_LEN - (skillHeader + angleLen));
    if (CMD_LEN > spaceAfterStoringData) {
      PTF("LMT ");
      PTL(spaceAfterStoringData);
    }
    for (int i = 0; i <= angleLen; i++)
      newCmd[BUFF_LEN - i] = newCmd[skillHeader + angleLen - i];
    dutyAngles = (int8_t *)newCmd + BUFF_LEN - angleLen;
  }

  void formatSkill() {
    transformSpeed = 1;  // period > 1 ? 1 : 0.5;
    firstMotionJoint = (period <= 1) ? 0 : DOF - WALKING_DOF;

    for (int i = 0; i < 2; i++) {
      expectedRollPitch[i] = (int8_t)newCmd[1 + i];
#ifdef GYRO_PIN
      yprTilt[2 - i] = 0;
#endif
    }
    angleDataRatio = (int8_t)newCmd[3];
    byte baseHeader = 4;
    if (period < 0) {
      for (byte i = 0; i < 3; i++)
        loopCycle[i] = (int8_t)newCmd[baseHeader++];
    }
    inplaceShift();
    periodGlobal = period;
    // int len = abs(period) * frameSize;
    // dutyAngles = new int8_t[len];
    // for (int k = 0; k < abs(period); k++) {
    //   for (int col = 0; col < frameSize; col++) {
    //     if (WALKING_DOF == 12 && GAIT_ARRAY_DOF == 8 && period > 1)
    //       if (col < 4)
    //         dutyAngles[k * frameSize + col] = 0;
    //       else
    //         dutyAngles[k * frameSize + col] = int8_t(newCmd[skillHeader + k * GAIT_ARRAY_DOF + col - 4]);
    //     else
    //       dutyAngles[k * frameSize + col] = int8_t(newCmd[skillHeader + k * frameSize + col]);
    //   }
    // }
  }
#define PRINT_SKILL_DATA
  void info() {
    PT("Skill Name: ");
    PTL(skillName);
    PTF("period: ");
    PT(period);
    PT(",\texpected(pitch,roll): (");
    PT(expectedRollPitch[0]);
    PT(",");
    PT(expectedRollPitch[1]);
    PT(")\t");
    PTF("angleRatio: ");
    PTL(angleDataRatio);
    if (period < 0) {
      PT("loop frame: ");
      for (byte i = 0; i < 3; i++)
        PT(String((byte)loopCycle[i]) + ", ");
      PTL();
    }
#ifdef PRINT_SKILL_DATA
    int showRows = 1;
    for (int k = 0; k < abs(period); k++) {
      if (abs(period) <= showRows + 2 || k < showRows || k == abs(period) - 1) {
        for (int col = 0; col < frameSize; col++) {
          PT((int8_t)dutyAngles[k * frameSize + col]);
          PT(",\t");
        }
        PTL();
      } else {
        if (k == showRows) {
          PTF(" skipping ");
          PT(abs(period) - 1 - showRows);
          PTF(" frames");
        }
        PT('.');
        if (k == abs(period) - 2)
          PTL();
      }
    }
#endif
    PTL();
  }
  void mirror() {  // Create a mirror function to allow the robot to pick random directions of behaviors.
    // It makes the robot more unpredictable and helps it get rid of an infinite loop,
    // such as failed fall-recovering against a wall.
    expectedRollPitch[0] = -expectedRollPitch[0];
    for (int k = 0; k < abs(period); k++) {
      if (period <= 1) {                                         // behavior
        dutyAngles[k * frameSize] = -dutyAngles[k * frameSize];  // head and tail panning angles
#ifndef ROBOT_ARM                                                // avoid mirroring the pincers' movements
        dutyAngles[k * frameSize + 2] = -dutyAngles[k * frameSize + 2];
#endif
      }
      for (byte col = (period > 1) ? 0 : 2; col < ((period > 1) ? WALKING_DOF : DOF) / 2; col++) {
        int8_t temp = dutyAngles[k * frameSize + 2 * col];
        dutyAngles[k * frameSize + 2 * col] = dutyAngles[k * frameSize + 2 * col + 1];
        dutyAngles[k * frameSize + 2 * col + 1] = temp;
      }
    }
  }
  void shiftCenterOfMass(int angle) {
    int offset = 8;
    if (period > 1)
      offset = 0;
    for (int k = 0; k < abs(period); k++) {
      // printList(dutyAngles + k * frameSize, frameSize); //compare the angle change
      float rate = 1.2;
      if (angle < 0)
        rate = 0.6;
      for (byte col = 0; col < 2; col++) {
        dutyAngles[k * frameSize + offset + col] = dutyAngles[k * frameSize + offset + col] + angle;
      }
      for (byte col = 4; col < 6; col++) {
        dutyAngles[k * frameSize + offset + col] = dutyAngles[k * frameSize + offset + col] - angle * rate;
      }
      // printList(dutyAngles + k * frameSize, frameSize);
    }
  }
  int nearestFrame() {
    if (period == 1)
      frame = 0;
    else  // find the nearest frame using certain algorithm
      frame = 0;
    return frame;
  }
  void transformToSkill(int frame = 0) {
    //      info();
    transform(dutyAngles + frame * frameSize, angleDataRatio, transformSpeed, firstMotionJoint, period, runDelay);
  }
  void convertTargetToPosture(int *targetFrame) {
    int extreme[2];
    getExtreme(targetFrame, extreme);
    if (extreme[0] < -125 || extreme[1] > 125) {
      angleDataRatio = 2;
      for (int i = 0; i < DOF; i++)
        targetFrame[i] /= 2;
    } else
      angleDataRatio = 1;
    arrayNCPY(dutyAngles, targetFrame, DOF);
    period = 1;
    firstMotionJoint = 0;
    frameSize = DOF;
    frame = 0;
  }
  void perform() {
    if (period < 0) {  // behaviors
      interruptedDuringBehavior = false;
      int8_t repeat = loopCycle[2] >= 0 && loopCycle[2] < 2 ? 0 : loopCycle[2] - 1;
      bool gyroBalanceQlag = gyroBalanceQ;
      gyroBalanceQ = strcmp(skillName, "bf") && strcmp(skillName, "ff") && strcmp(skillName, "flipF") && strcmp(skillName, "flipD") && strcmp(skillName, "flipL") && strcmp(skillName, "flipR") && strcmp(skillName, "pd") && strcmp(skillName, "hds") && strcmp(skillName, "bx") && strstr(skillName, "rl") == NULL;  // won't read gyro for fast motion
      for (byte c = 0; c < abs(period); c++) {                                                                                                                                                                                                                                                                         // the last two in the row are transition speed and delay
        Stream *serialPort = NULL;
        String source;
#ifdef BT_SSP
        if (SerialBT.available()) {  // give BT a higher priority over wired serial
          serialPort = &SerialBT;
          source = "BT";
        } else
#endif
#ifdef VOICE
          if (SERIAL_VOICE.available()) {
          serialPort = &SERIAL_VOICE;
          source = "Voice";
        } else
#endif
          // the BT_BLE is unhandled here
          if (moduleActivatedQ[0] && Serial2.available()) {
            serialPort = &Serial2;
            source = "Serial2";
          } else if (Serial.available()) {
            serialPort = &Serial;
            source = "Serial";
          }
        if (serialPort                                                                     // user input
            || (gyroBalanceQ                                                               // the IMU should be used for balancing
                && ((imuException != IMU_EXCEPTION_FLIPPED && !strcmp(skillName, "rc"))    // recovered during recover
                    || (imuException == IMU_EXCEPTION_FLIPPED && strcmp(skillName, "rc"))  // flipped during other skills
                    ))) {
#ifdef GYRO_PIN
          print6Axis();
#endif
          PTHL("imuException: ", imuException);
          PTLF("Behavior interrupted");
          interruptedDuringBehavior = true;
          return;
        }
        // printToAllPorts("Progress: " + String(c + 1) + "/" + abs(period));
        //  printList(dutyAngles + c * frameSize);
        transform(dutyAngles + c * frameSize, angleDataRatio, dutyAngles[DOF + c * frameSize] / 8.0);
#ifdef GYRO_PIN  // if opt out the gyro, the calculation can be really fast
        if (dutyAngles[DOF + 2 + c * frameSize]) {
          int triggerAxis = dutyAngles[DOF + 2 + c * frameSize];
          int triggerAngle = dutyAngles[DOF + 3 + c * frameSize];
          float currentYpr = ypr[abs(triggerAxis)];
          float previousYpr = currentYpr;
          long triggerTimer = millis();
          while (1) {
            // readIMU();
            print6Axis();
            currentYpr = ypr[abs(triggerAxis)];
            // PT(currentYpr);
            // PTF("\t");
            // PTL(triggerAngle);
            if (
              ((180 - fabs(currentYpr) > 2)                                                                                           // skip the angle when the reading jumps from 180 to -180
               && (triggerAxis * currentYpr > triggerAxis * triggerAngle && triggerAxis * previousYpr < triggerAxis * triggerAngle))  // the sign of triggerAxis will deterine whether the current angle should be larger or smaller than the trigger angle
              || millis() - triggerTimer > 2000) {                                                                                    // if the robot stucks by the trigger for more than 3 seconds, it will break.
              PT(previousYpr);
              PT(" => ");
              PT(triggerAngle);
              PT(" => ");
              PTL(currentYpr);
              PTLF("Trigger released");
              break;
            }
            previousYpr = currentYpr;
          }
        }
#endif
        delay(abs(dutyAngles[DOF + 1 + c * frameSize] * 50));

        if (repeat != 0 && c != 0 && c == loopCycle[1]) {
          // printToAllPorts("Loop remaining: " + String(repeat));
          c = loopCycle[0] - 1;
          if (repeat > 0)  // if repeat <0, infinite loop. only reset button will break the loop
            repeat--;
        }
      }
      gyroBalanceQ = gyroBalanceQlag;
      // printToAllPorts(token); // avoid printing the token twice. may be introduced to fix some other issues.
    } else {  // postures and gaits
#ifdef GYRO_PIN
      if (imuUpdated && gyroBalanceQ && !(frame % imuSkip)) {
        //          PT(ypr[2]); PT('\t');
        //          PT(RollPitchDeviation[0]); PT('\t');
        //          printList(currentAdjust);
        for (byte i = 0; i < 2; i++) {
          RollPitchDeviation[i] = ypr[2 - i] - expectedRollPitch[i];                                                                          // all in degrees
          RollPitchDeviation[i] = sign(ypr[2 - i]) * max(float(fabs(RollPitchDeviation[i]) - levelTolerance[i]), float(0)) + yprTilt[2 - i];  // filter out small angles
        }
        imuUpdated = false;
      }
#endif
      for (int jointIndex = 0; jointIndex < DOF; jointIndex++) {
        //          PT(jointIndex); PT('\t');
// #ifdef ROBOT_ARM
//         if (abs(period) > 1 && jointIndex == 0)  //don't move the robot arm's joints for gaits
//           jointIndex = 4;
// #endif
#ifndef HEAD
        if (jointIndex == 0)
          jointIndex = 2;
#endif
#ifndef TAIL
        if (jointIndex == 2)
          jointIndex = DOF - WALKING_DOF;
#endif
#if WALKING_DOF == 8
        if (jointIndex == 4)
          jointIndex = 8;
#endif
        //          PT(jointIndex); PT('\t');
        float duty;
        if ((abs(period) > 1 && jointIndex < firstMotionJoint)      // gait and non-walking joints
            || (abs(period) == 1 && jointIndex < 4 && manualHeadQ)  // posture and head group and manually controlled head
        ) {
          if (!manualHeadQ && jointIndex < 4) {
#ifndef ROBOT_ARM
            duty =
              (jointIndex != 1 ? offsetLR : 0)  // look left or right
              + 10 * sin(frame * (jointIndex + 2) * M_PI / abs(period));
#else
            if (jointIndex == 1 && strstr(skillName, "bk") != NULL)
              duty = 50;
            else
              duty = 0;
#endif
          } else
            duty = currentAng[jointIndex] + max(-20, min(20, (targetHead[jointIndex] - currentAng[jointIndex])));
          //  - gyroBalanceQ * currentAdjust[jointIndex];
        } else {
          duty = dutyAngles[frame * frameSize + jointIndex - firstMotionJoint] * angleDataRatio;
        }
        duty =
#ifdef GYRO_PIN
          +gyroBalanceQ * ((!imuException || imuException == IMU_EXCEPTION_LIFTED) ?  // not exception or the robot is lifted
                             (!(frame % imuSkip) ? adjust(jointIndex, (period == 1)) : currentAdjust[jointIndex])
                                                                                   : 0)
            / (!fineAdjustQ && !mpuQ ? 4 : 1)  // reduce the adjust if not using mpu6050
#endif
          + duty;
        calibratedPWM(jointIndex, duty);
      }
      frame += tStep;
      if (frame >= abs(period)) {
        frame = 0;
        
        // Check if in cycle counting mode and count completed cycles
        if (cycleCountingMode && period > 1) {
          completedCycles++;
          PTH("Completed cycle: ", completedCycles);
          PTHL(" / ", targetCycles);
          
          if (completedCycles >= targetCycles) {
            // Target cycles reached, stop the gait
            cycleCountingMode = false;
            completedCycles = 0;
            targetCycles = 0;
            tQueue->addTask('k', "up");
            PTLF("Cycle target reached, stopping gait");
          }
        }
      }
    }
  }
};
Skill *skill;

void loadBySkillName(const char *skillName) {  // get lookup information from on-board EEPROM and read the data array from storage
  char lr = skillName[strlen(skillName) - 1];
  int skillIndex;
#ifdef ROBOT_ARM  // use the altered Arm gait
  bool optimizedForArm = false;
  char *nameStr = new char[strlen(skillName) + 4];
  strcpy(nameStr, skillName);
  if (lr == 'L' || lr == 'R' || lr == 'F' && strstr(nameStr, "Arm") == NULL) {  // try to find the arm version
    // if the name contains L R F and doesn't contain "Arm"
    nameStr[strlen(skillName) - 1] = '\0';  // remove the L R F in the end
    strcat(nameStr, "Arm");                 // insert Arm
    nameStr[strlen(skillName) + 2] = lr;    // append L R F
    nameStr[strlen(skillName) + 3] = '\0';
    // PTHL("mod ", nameStr);
  }
  skillIndex = skillList->lookUp(nameStr);
  if (skillIndex != -1)
    optimizedForArm = true;
  else {
    optimizedForArm = false;  // if there's no special skillname with Arm, use the original skill
    skillIndex = skillList->lookUp(skillName);
  }
#else
  skillIndex = skillList->lookUp(skillName);
#endif
  if (skillIndex != -1) {
    // if (skill != NULL)
    //   delete[] skill;

    skill->offsetLR = (lr == 'L' ? 30 : (lr == 'R' ? -30 : 0));
    skill->buildSkill(skillList->get(skillIndex)->index);
    strcpy(newCmd, skill->skillName);
#ifdef GYRO_PIN
    // keepDirectionQ = (skill->period > 1) ? false : true;
    thresX = (skill->period > 1) ? 12000 : 5000;
    thresY = (skill->period > 1) ? 10000 : 4000;
    thresZ = (skill->period > 1) ? 15000 : 12000;
#endif
    if (strcmp(newCmd, "calib") && skill->period == 1) {      // for static postures
      int8_t protectiveShift = esp_random() % 60 / 10.0 - 3;  // +- 3.0 degrees
      for (byte i = 0; i < DOF; i++)
#ifdef ROBOT_ARM
        if (i != 2)
#endif
          skill->dutyAngles[i] += protectiveShift;  // add protective shift to reduce wearing at the same spot
    }
    // skill->info();
    if (lr == 'R'                                                 // 'R' must mirror
        || ((lr == 'X' || lr != 'L')                              // 'L' should not mirror
            && ((random(10) > 7 && random(10) > 5) || coinFace))  // 1/5 chance to random otherwise flip everytime
    )
      skill->mirror();  // mirror the direction of a behavior
    coinFace = !coinFace;
#ifdef ROBOT_ARM
    if (skill->period == 1 && strcmp(newCmd, "calib")  // postures
        || skill->period > 1 && !optimizedForArm)      // gaits
      skill->shiftCenterOfMass(-10);
#endif
    skill->transformToSkill(skill->nearestFrame());
    // #ifdef NYBBLE
    for (byte i = 0; i < HEAD_GROUP_LEN; i++)
      targetHead[i] = currentAng[i] - currentAdjust[i];
    // #endif
    //    runDelay = delayMid + 2;
    // skill->info();
  }
}
