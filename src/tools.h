#ifndef TOOLS_H
#define TOOLS_H

// Serial Print macro directive abbreviations
// Simple macro directives
#define PT(s)             Serial.print(s)          // Serial print
#define PTD(s, format)    Serial.print(s, format)  // Formatted serial print
#define PTL(s)            Serial.println(s)        // Serial print plus newline
#define PTF(s)            Serial.print(F(s))       // Serial print plus newline with trading flash memory for dynamic memory using F() function
#define PTLF(s)           Serial.println(F(s))     // Serial print plus newline with trading flash memory for dynamic memory using F() function

  // Composite macro directives
#define PTT(s, delimeter)   {PT(s);    PT(delimeter);}               // Serial print with delimiter
#define PTTL(s, delimeter)  {PT(s);    PTL(delimeter);}              // Serial print with delimiter plus newline
#define PTH(head, value)    {PT(head); PT('\t');       PT(value);}   // Serial print with head, tab, value
#define PTHL(head, value)   {PT(head); PT('\t');       PTL(value);}  // Serial print with head, tab, value plus newline

char getUserInputChar(int waitTimeout = 0) {  //take only the first character, allow "no line ending", "newline", "carriage return", and "both NL & CR"
  long start = millis();
  if (waitTimeout) {
    PTF("(Auto skip in ");
    PT(waitTimeout);
    PTLF(" seconds)");
  }
  int timeLeft = waitTimeout;
  while (!Serial.available()) {
    long current = millis();
    if (waitTimeout) {
      if ((current - start) / 1000 > waitTimeout)
        return 'n';
      if (waitTimeout - timeLeft < (current - start) / 1000) {
        PT(timeLeft--);
        PT("...");
      }
    }
  }
  char result = Serial.read();
  delay(1);                     //wait for the remainder to arrive
  while (Serial.available()) {  //flush the '\r' or '\n' if any
    Serial.read();
  }
  return result;
}

//template <typename T> int8_t sign(T val) {
//  return (T(0) < val) - (val < T(0));
//}

void printRange(int r0 = 0, int r1 = 0) {
  if (r1 == 0)
    for (byte i = 0; i < r0; i++) {
      PT(i);
      PT('\t');
    }
  else
    for (byte i = r0; i < r1; i++) {
      PT(i);
      PT('\t');
    }
  PTL();
}

String range2String(int r0 = 0, int r1 = 0) {
  String temp = "";
  if (r1 == 0)
    for (byte i = 0; i < r0; i++) {
      temp += i;
      temp += '\t';
    }
  else
    for (byte i = r0; i < r1; i++) {
      temp += i;
      temp += '\t';
    }
  return temp;
}

template<typename T> void printList(T *arr, byte len = DOF) {
  String temp = "";
  for (byte i = 0; i < len; i++) {
    temp += String(int(arr[i]));
    temp += ",\t";
    //PT((T)(arr[i]));
    //PT('\t');
  }
  PTL(temp);
}

template<typename T> String list2String(T *arr, byte len = DOF) {
  String temp = "";
  for (byte i = 0; i < len; i++) {
    temp += String(int(arr[i]));  //String(int(arr[i]));
    temp += ",\t";
    //PT((T)(arr[i]));
    //PT('\t');
  }
  return temp;
}

template<typename T> void printTable(T *list) {
  printRange(0, DOF);
  printList(list, DOF);
}

template<typename T> int strlenUntil(T *s, char terminator) {
  int l = 0;
  while (l < BUFF_LEN && s[l] != terminator) {
    l++;
  }
  return l;
}

template<typename T> void printListWithoutString(T *arr, byte len = DOF) {
  for (byte i = 0; i < len; i++) {
    PT((T)(arr[i]));
    if (!(i + 1) % 20)
      PTL();
    else
      PT(",\t");
  }
  PTL();
}

template<typename T> void printCmdByType(char t, T *data) {
  if (t != '\0') {
    PT("token ");
    PT(t);
    int len = (t < 'a') ? strlenUntil(data, '~') : strlen((char *)data);
    PT("\tcalculated len: ");
    PTL(len);
    if (t < 'a') {
      PTL("Binary ");
      printListWithoutString((int8_t *)data, len);
    } else {
      PT("ASCII ");
      PTL((char *)data);
    }
  }
}

// template<typename T, typename T1> void arrayNCPY(T *destination, const T1 *source, int len) {  //deep copy regardless of '\0'
//   for (int i = 0; i < len; i++)
//     destination[i] = min((T1)125, max((T1)-125, source[i]));
// }
template<typename T, typename T1> void arrayNCPY(T *destination, const T1 *source, int len) {  //deep copy regardless of '\0'
  for (int i = 0; i < len; i++) {
    // destination[i] = min((T1)125, max((T1)-125, source[i]));
    // PT(destination[i]);
    // PT('\t');
    // PT(source[i]);
    destination[i] = source[i];
  }
}

template<typename T> void getExtreme(T *arr, T *extreme, int len = DOF) {
  extreme[0] = 128;   //min
  extreme[1] = -127;  // max
  for (int i = 0; i < len; i++) {
    if (arr[i] < extreme[0])
      extreme[0] = arr[i];
    if (arr[i] > extreme[1])
      extreme[1] = arr[i];
  }
}

void FPS() {
  if (millis() - loopTimer < 1000)
    fps++;
  else {
    PTHL("FPS:", fps);
    // PTL(fps);
    fps = 0;
    loopTimer = millis();
  }
}

void leftTrimSpaces(char *s, int *len) {
  char *head = s;
  while (*head == ' ' || *head == '\t') {
    head++;
    (*len)--;
  }
  strcpy(s, head);
}

void printCmd() {
  PTF("lastT:");
  PT(lastToken);
  PTF("\tT:");
  PT(token);
  PTF("\tLastCmd:");
  PT(lastCmd);
  PTF("\tCmd:");
  printCmdByType(token, newCmd);
}

void resetCmd() {
  // PTL("Reset Cmd");
  // printCmd();
  
  lastToken = token;
  newCmdIdx = 0;
  if (token != T_SKILL && token != T_SKILL_DATA && token != T_SERVO_CALIBRATE && token != T_SERVO_FEEDBACK && token != T_SERVO_FOLLOW && token != T_CPG && token != T_CPG_BIN)
    token = '\0';
  newCmd[0] = '\0';
  cmdLen = 0;
  // PTL("Done Reset");
}

char *strGet(char *s, int i) {  //allow negative index
  int len = strlen(s);
  if (abs(i) <= len)
    return s + (i < 0 ? len + i : i);
  else {
    PTL("Invalid index!");
    return s;
  }
}
#endif
