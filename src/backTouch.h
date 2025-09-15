int8_t touchPadMap[] = { 1, 3, 4, 2 };
String touchLocation[] = { "Front Left", "Front Right", "Center", "Back" };

void backTouchSetup() {
  pinMode(BACKTOUCH_PIN, INPUT);
}
int8_t prevTouch = -1;
long lastTouchEvent;
int8_t touchPadIdx;
int8_t backTouchID() {
  int touchReading = analogRead(BACKTOUCH_PIN);
  // PTT(touchReading, '\t');
  if (touchReading < 3000)
    return touchPadMap[touchReading / 600] ;
  else
    return 0;
}
void read_backTouch() {
  // put your main code here, to run repeatedly:
  //stats();
  int touchPadIdx = backTouchID();
  // PTL(touchPadIdx);

  if (touchPadIdx) {
    if (prevTouch != touchPadIdx) {  // || millis() - lastTouchEvent > 500) {  // if the touch is different or a repeatitive touch interval is longer than 0.5 second
      if(prevTouch == -1){ // avoid head motion if backtouch is not connected
        prevTouch = touchPadIdx;
        return;
      }
      prevTouch = touchPadIdx;
      PTHL("Touched:", touchLocation[touchPadIdx - 1]);
      beep(touchPadIdx * 2 + 15, 100);
      switch (touchPadIdx) {
        case 1:
          {
#ifdef ROBOT_ARM
            tQueue->addTask('m', "0,45,1,45,2,0");
#else
            tQueue->addTask('i', "1,-30,0,120", 1000);
            tQueue->addTask('m', "0,0,1,0");
#endif
            break;
          }
        case 2:
          {
#ifdef ROBOT_ARM
            tQueue->addTask('m', "0,-45,1,45,2,0", 1000);
#else
            tQueue->addTask('i', "1,-30,0,-120,", 1000);
            tQueue->addTask('m', "0,0,1,0");
#endif
            break;
          }
        case 3:
          {
#ifdef ROBOT_ARM
            tQueue->addTask('m', "0,0,1,0,2,0", 1000);
#else
            tQueue->addTask('k', "str", 1000);
            tQueue->addTask('k',"up");
#endif
            break;
          }
        case 4:
          {
#ifdef ROBOT_ARM
            tQueue->addTask('m', "1,60,2,120", 1000);
#else
            tQueue->addTask('k', "buttUp", 0);
#endif

#ifdef NYBBLE
            tQueue->addTask('m', "2,-30,2,30,2,0", 500);
#endif
            break;
          }
      }
      // tQueue->addTask('k', "up");
      lastTouchEvent = millis();
    }
  }
}
