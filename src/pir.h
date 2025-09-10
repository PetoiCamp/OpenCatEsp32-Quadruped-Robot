#ifdef BiBoard_V1_0
#define PIR_PIN ANALOG2
#else 
#define PIR_PIN ANALOG1
#endif
bool previousPIR = false;

void createPirTask() {  //this is an example task
  tQueue->addTask('k', "sit");
  tQueue->addTask('m', "0 -60 0 60", 2000);
  char music[] = { 14, 8, 18, 16, 16, 16, 13, 16, 9, 16, 11, 16, 0, 4, 13, 8, 9, 4, '~' };
  tQueue->addTask('B', music);
  tQueue->addTask('k', "str", 2000);
  tQueue->addTask('k', "up");
}

void pirSetup() {
  pinMode(PIR_PIN, INPUT);
}
void read_PIR() {
  bool currentPIR = analogRead(PIR_PIN) > 3000;
  PTL(currentPIR);
  if (currentPIR && !previousPIR) {
    createPirTask();
    previousPIR = 1;
  } else if (!currentPIR && previousPIR) {
    newCmdIdx = 5;
    token = T_REST;
    previousPIR = 0;
  }
}
