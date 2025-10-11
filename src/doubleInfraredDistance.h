/*
Demo for the Petoi double infrared distance sensor
It works under the indoor lighting condition.

The robot will make different sound by the distance of the obstable in front of it.

When the robot is in static posture:
1. It will turn its head to the object in front of it.
2. When the view angle is larger than 75 degrees, the robot will turn its body to that direction.
3. If the object is too close, the robot will walk backward then walk forward, then sit.

When the robot is walking:
1. It will trot forward when the front is clear.
2. It will turn when there's an obstacle within 30cm.
3. It will retreat and turn if the obstacle is within 12cm.
4. It will rotate when the obstacle is within 4cm.

Rongzhong Li
Petoi LLC
2023 April 17
*/
#define READING_COUNT 30
#define SENSOR_DISPLACEMENT 3.7

// #ifdef NyBoard_V1_0
// #define NEOPIXEL_PIN 10 // the code for NeoPixels have to be shrinked to fit in the board
// #define NUMPIXELS 7
// #elif defined NyBoard_V1_2
// #define NEOPIXEL_PIN 10 // the code for NeoPixels have to be shrinked to fit in the board
// #define NUMPIXELS 1
// #endif

float fit(int d)
{
  if (d < 130)
    return d / 8.0;
  else
    return 4096.0 / (pow(4096 - d, 1.0 / 3) + 25) - 84;
}

#ifdef NEOPIXEL_PIN
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
#endif

bool makeSound = true;

// Debug switch: print IR distances and thresholds (enable by defining DEBUG_IR_DISTANCE)

float kpDistance = 0.24;    // Proportional gain (faster response)
float kiDistance = 0.0;     // Integral gain (disable to avoid oscillation)
float kdDistance = 0.22;    // Derivative gain
float setpoint = 0;         // Target value
float errorDistance = 0;    // Difference between setpoint and actual value
float integral = 0;         // Running sum of errors over time
float derivative = 0;       // Rate of change of errorDistance over time
float last_error = 0;       // errorDistance in the previous iteration
float currentXDistance = 0; // Control signal sent to the sensors

// --- IR behavior mode ---
enum { IR_SIT_TRACK = 0, IR_WALK_AVOID = 1 };
volatile uint8_t irBehaviorMode = IR_SIT_TRACK; // default: sit and head-only tracking

// Forward declaration for setIRBehaviorMode
void resetPID();

void setIRBehaviorMode(uint8_t mode)
{
  static uint8_t lastMode = 255;
  irBehaviorMode = mode;
  if (irBehaviorMode != lastMode)
  {
    if (mode == IR_SIT_TRACK)
    {
      // Switch to static tracking: sit and hand over to head PID
      if (tQueue)
        tQueue->addTask('k', "sit");
      // Add an empty task to avoid conflict between performSkill and PID loop
      if (tQueue)
        tQueue->addTask('i', "");
      manualHeadQ = true;
    }
    else if (mode == IR_WALK_AVOID)
    {
      // Switch to walking/avoidance: handled by gait branch. Do not force start walking here.
    }
    resetPID();
    lastMode = irBehaviorMode;
  }
}

float d = SENSOR_DISPLACEMENT; // Displacement of sensors on the x-axis
int rawL, rawR;
float dL, dR, meanD, maxD, minD;
int meanA = 0, meanB = 0, diffA_B = 0, actualDiff = 0, last = 0;
int longThres = 20;
long leftCloseTimer, rightCloseTimer;
const float sitLongThres = 45.0; // sit-mode tracking distance threshold (cm)

// --- Anti-oscillation parameters (adapted from IRCatcher) ---
float deadzone = 3.0;          // main deadzone (cm) - larger to suppress micro jitters
float fineDeadzone = 1.2;      // far-distance fine deadzone (cm)
float error_threshold = 2.0;   // boundary between small/large error (cm)

// Diff anti-jump filtering
float filteredDistanceDiff = 0;
float lastDistanceDiff = 0;
const float DIFF_FILTER_ALPHA = 0.3;        // diff filter alpha
const float MAX_DIFF_CHANGE = 8.0;          // max diff change per cycle (cm)
const float SMALL_OBJECT_THRESHOLD = 15.0;  // small-object threshold (cm)

// 大误差区PID参数(距离自适应用)
float kpDistanceLarge = 0.6;
float kiDistanceLarge = 0.006;
float kdDistanceLarge = 0.18;

// Derivative low-pass filter and output throttle
const float DERIV_ALPHA = 0.2; // derivative LPF alpha (0-1), smaller = smoother
static float derivFiltered = 0;
static float lastErrorRaw = 0;
static unsigned long lastCmdMs = 0; // 上次PWM输出时间

// Sit-mode hold hysteresis (suppress jitter near center)
const float SIT_HOLD_DEADZONE = 0.7;       // enter-hold error threshold (cm)
const float SIT_RELEASE_THRESHOLD = 1.4;   // release-hold error threshold (cm)
const float SIT_DERIV_THRESHOLD = 0.25;    // error-change threshold (cm/cycle)

// Small-object/single-side "background hit" suppression
const float BG_MARGIN = 3.0;               // safety margin for background threshold (cm)
const float NEAR_SINGLE_DISTANCE = 25.0;   // near-side threshold (cm)
#ifdef ROBOT_ARM
const float SINGLE_STEP_SIT = 2;        // single-step (deg) in sit mode - ARM form
const float SINGLE_STEP_WALK = 4;       // single-step (deg) in walk mode - ARM form
#else
const float SINGLE_STEP_SIT = 4;        // single-step (deg) in sit mode - non-ARM doubled (incl. Nybble/Bittle)
const float SINGLE_STEP_WALK = 8;       // single-step (deg) in walk mode - non-ARM doubled (incl. Nybble/Bittle)
#endif

static inline float backgroundThreshold()
{
  // Calculate background threshold from max raw reading with a safety margin
  float farD = fit(4095);
  return max(50.0f, farD - BG_MARGIN); // 不低于50cm，避免过近也被当作背景
}

static inline float calculateDistanceFactor(float averageDistance)
{
  float distanceFactor = 1.0;
  if (averageDistance <= 15.0)
    distanceFactor = 0.8;
  else if (averageDistance <= 30.0)
    distanceFactor = 0.8 + (averageDistance - 15.0) / 15.0 * 0.2; // 0.8->1.0
  else if (averageDistance >= 80.0)
    distanceFactor = 1.1; // 远距离略增强
  else
    distanceFactor = 1.0 + (averageDistance - 30.0) / 50.0 * 0.1; // 1.0->1.1
  return distanceFactor;
}

static inline float processDistanceDiffWithAntiJump(float rawDiff, float averageDistance)
{
  static bool initialized = false;
  if (!initialized)
  {
    filteredDistanceDiff = rawDiff;
    lastDistanceDiff = rawDiff;
    initialized = true;
    return rawDiff;
  }

  float diffChange = fabs(rawDiff - lastDistanceDiff);
  bool isSmallObject = averageDistance < SMALL_OBJECT_THRESHOLD;

  float processedDiff = rawDiff;
  if (isSmallObject && diffChange > MAX_DIFF_CHANGE)
  {
    float maxChange = MAX_DIFF_CHANGE;
    processedDiff = (rawDiff > lastDistanceDiff) ? (lastDistanceDiff + maxChange)
                                                : (lastDistanceDiff - maxChange);
  }

  float filterAlpha = isSmallObject ? DIFF_FILTER_ALPHA * 0.5 : DIFF_FILTER_ALPHA;
  filteredDistanceDiff = filterAlpha * processedDiff + (1 - filterAlpha) * filteredDistanceDiff;
  lastDistanceDiff = filteredDistanceDiff;
  return filteredDistanceDiff;
}

void resetPID()
{
  errorDistance = 0;
  last_error = 0;
  integral = 0;
  derivative = 0;
}

void distanceNaive(float dLeft, float dRight)
{ // a simple feedback loop without PID
  float minD = min(dLeft, dRight);
  float diff = dRight - dLeft;
  float offset = atan(diff / SENSOR_DISPLACEMENT) * degPerRad;

  if (periodGlobal == 1)
  { // posture
    if (minD < 20 && abs(offset) > 5 - minD / 5)
    {
      // PT("\tmin ");
      // PT(minD);
      // PT("\tdiff ");
      // PT(diff);
      // PT("\toffset ");
      // PT(offset);
      currentXDistance = min(90.0, max(-90.0, double(currentXDistance + offset / 10)));
      calibratedPWM(0, currentXDistance, 0.2);
      // PTL();
#ifdef SHOW_FPS
      FPS();
#endif
    }
  }
}

void distancePID(float dLeft, float dRight)
{
  // In sit mode allow longer tracking range; keep short range for walking
  float minDLocal = min(dLeft, dRight);
  float allowedThres = (irBehaviorMode == IR_SIT_TRACK) ? sitLongThres : longThres;
  if (minDLocal < allowedThres && dL != 200 && dR != 200)
  {
    // Compute average distance and diff; apply anti-jump filter
    float averageDistance = (dLeft + dRight) / 2.0;
    float rawDiff = dLeft - dRight - setpoint;
    float diff = processDistanceDiffWithAntiJump(rawDiff, averageDistance);

    // Background detection
    float BG_DISTANCE = backgroundThreshold();
    // Both sides background: target far away. Center head and reset PID states
    bool bothBackground = (dLeft >= BG_DISTANCE) && (dRight >= BG_DISTANCE);
    if (bothBackground)
    {
      bool sitMode = (irBehaviorMode == IR_SIT_TRACK);
#ifdef DEBUG_IR_DISTANCE
      PT("both-side bg | L:"); PT(dLeft); PT(" R:"); PT(dRight); PTL();
#endif
      manualHeadQ = true;
      targetHead[0] = 0; // center
      if (targetHead[0] > 75) targetHead[0] = 75;
      if (targetHead[0] < -75) targetHead[0] = -75;
      currentXDistance = targetHead[0];

      unsigned long nowMs = millis();
      unsigned long throttleMs = sitMode ? 30 : 25;
      if (nowMs - lastCmdMs >= throttleMs)
      {
        int stepLimit = sitMode ? 8 : 8;
        int duty = currentAng[0] + max(-stepLimit, min(stepLimit, (targetHead[0] - currentAng[0])));
        calibratedPWM(0, duty, 0.5);
        lastCmdMs = nowMs;
      }

      // Reset PID integrator/derivative to avoid a jump when resuming PID
      resetPID();
      return;
    }

    // Single-side background hit: approach with small fixed steps
    bool leftIsBackground = (dLeft >= BG_DISTANCE) && (dRight <= NEAR_SINGLE_DISTANCE);
    bool rightIsBackground = (dRight >= BG_DISTANCE) && (dLeft <= NEAR_SINGLE_DISTANCE);
    if (leftIsBackground || rightIsBackground)
    {
      bool sitMode = (irBehaviorMode == IR_SIT_TRACK);
      float step = sitMode ? SINGLE_STEP_SIT : SINGLE_STEP_WALK;
      // If left is background and right is near: step right; vice versa step left
      float signedStep = rightIsBackground ? step : -step; // right bg -> 向右；left bg -> 向左
      #ifdef DEBUG_IR_DISTANCE
      PT("single-side bg | L:"); PT(dLeft); PT(" R:"); PT(dRight); PT(" step:"); PT(signedStep); PTL();
      #endif
      manualHeadQ = true;
      targetHead[0] = currentAng[0] + signedStep;
      if (targetHead[0] > 75) targetHead[0] = 75;
      if (targetHead[0] < -75) targetHead[0] = -75;
      currentXDistance = targetHead[0];

      // Update PID states to avoid a jump when returning to PID
      if (kiDistance > 0.0)
        integral = max(-150.0, min(150.0, double(integral + diff)));
      else
        integral = 0;
      float rawDerivativeBg = diff - last_error;
      {
        float derivAlphaBg = sitMode ? 0.18 : DERIV_ALPHA;
        derivFiltered = derivAlphaBg * rawDerivativeBg + (1 - derivAlphaBg) * derivFiltered;
        derivative = derivFiltered;
      }
      last_error = diff;

      unsigned long nowMs = millis();
      unsigned long throttleMs = sitMode ? 35 : 25;
      if (nowMs - lastCmdMs >= throttleMs)
      {
        int stepLimit = sitMode ? 4 : 6;
        int duty = currentAng[0] + max(-stepLimit, min(stepLimit, (targetHead[0] - currentAng[0])));
        calibratedPWM(0, duty, 0.5);
        lastCmdMs = nowMs;
      }
      // 单侧模式下不继续常规PID，避免因背景读数造成的过冲
      return;
    }

    // Adaptive deadzone: larger at near range, smaller at far range
    float currentDeadzone = deadzone;
    bool sitMode = (irBehaviorMode == IR_SIT_TRACK);
    if (averageDistance < 15.0)
      currentDeadzone = deadzone * 1.2; // ~3.0cm
    else if (averageDistance < 30.0)
      currentDeadzone = deadzone;       // 2.5cm
    else if (averageDistance < 50.0)
      currentDeadzone = deadzone * 0.6; // 1.5cm
    else
      currentDeadzone = fineDeadzone;   // 0.8cm

    // In sit mode slightly increase deadzone (smaller than before) to improve response
    if (sitMode)
      currentDeadzone += 0.25;

    // In deadzone: no action and clear integrator to suppress oscillation
    if (fabs(diff) < currentDeadzone)
    {
      integral = 0;
      last_error = 0;
      return;
    }

    // Sit-mode hold hysteresis: freeze output when near center and slow change
    static bool sitHold = false;
    if (sitMode)
    {
      float diffChange = fabs(diff - last_error);
      if (!sitHold)
      {
        if (fabs(diff) < SIT_HOLD_DEADZONE && diffChange < SIT_DERIV_THRESHOLD)
        {
          sitHold = true;
        }
      }
      else
      {
        if (fabs(diff) > SIT_RELEASE_THRESHOLD || diffChange > SIT_DERIV_THRESHOLD * 1.5)
        {
          sitHold = false;
        }
      }

      if (sitHold)
      {
        // Freeze output but keep currentXDistance for display
        manualHeadQ = true;
        targetHead[0] = currentAng[0];
        currentXDistance = targetHead[0];
        return;
      }
    }

    // Distance-adaptive PID gains
    float distanceFactor = calculateDistanceFactor(averageDistance);
    float abs_error = fabs(diff);
    float kp, ki, kd;
    if (abs_error > 5.0)
    {
      kp = kpDistanceLarge * 1.5 * distanceFactor;
      ki = kiDistanceLarge;
      kd = kdDistanceLarge * distanceFactor;
    }
    else if (abs_error > error_threshold)
    {
      kp = kpDistanceLarge * distanceFactor;
      ki = kiDistanceLarge;
      kd = kdDistanceLarge * distanceFactor;
    }
    else if (abs_error > 1.0)
    {
      float ratio = (abs_error - 1.0) / (error_threshold - 1.0);
      kp = (kpDistance + ratio * (kpDistanceLarge - kpDistance)) * distanceFactor;
      ki = kiDistance + ratio * (kiDistanceLarge - kiDistance);
      kd = (kdDistance + ratio * (kdDistanceLarge - kdDistance)) * distanceFactor;
    }
    else
    {
      kp = kpDistance * 0.8 * distanceFactor;
      ki = kiDistance * 0.5;
      kd = kdDistance * distanceFactor;
    }

    // In sit mode slightly reduce gains overall
    if (sitMode)
    {
      kp *= 0.85;
      kd *= 0.9;
    }

    // PID terms
    // Disable or hard-limit integrator to avoid wind-up oscillation
    if (ki > 0.0)
      integral = max(-150.0, min(150.0, double(integral + diff)));
    else
      integral = 0;
    // 导数项加低通滤波，去抖
    float rawDerivative = diff - last_error;
    float derivAlpha = sitMode ? 0.18 : DERIV_ALPHA; // stronger smoothing in sit mode
    derivFiltered = derivAlpha * rawDerivative + (1 - derivAlpha) * derivFiltered;
    derivative = derivFiltered;
    last_error = diff;

    // Compute per-step angle adjustment (not absolute), avoid large swings
    float angleAdjustment = -(kp * diff + ki * integral + kd * derivative);
    float maxAdjustment;
    if (sitMode)
      maxAdjustment = (averageDistance < 20.0) ? 0.5 : 1.0; // 坐姿适度放宽
    else
      maxAdjustment = (averageDistance < 20.0) ? 0.6 : 1.2; // walking prep
    if (angleAdjustment > maxAdjustment) angleAdjustment = maxAdjustment;
    if (angleAdjustment < -maxAdjustment) angleAdjustment = -maxAdjustment;

    manualHeadQ = true;
    targetHead[0] = currentAng[0] + angleAdjustment;
    // Constrain head range
    if (targetHead[0] > 75) targetHead[0] = 75;
    if (targetHead[0] < -75) targetHead[0] = -75;
    // Keep compatibility for visualization/debug
    currentXDistance = targetHead[0];

    // Output throttle: 22ms (sit), 20ms (walk prep)
    unsigned long nowMs = millis();
    unsigned long throttleMs = sitMode ? 22 : 20;
    if (nowMs - lastCmdMs >= throttleMs)
    {
      // Slow follow steps
      int stepLimit = sitMode ? 8 : 8;
      int duty = currentAng[0] + max(-stepLimit, min(stepLimit, (targetHead[0] - currentAng[0])));
      calibratedPWM(0, duty, 0.5);
      lastCmdMs = nowMs;
    }
  }
}

void doubleInfraredDistanceSetup()
{
// put your setup code here, to run once:
#ifdef NEOPIXEL_PIN
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
#endif
#ifdef LED_PIN
  pinMode(LED_PIN, OUTPUT);
#endif
  manualHeadQ = true;
}

void readDistancePins()
{
  // #ifdef BiBoard_V1_0
  //   rawL = analogRead(ANALOG4);
  //   rawR = analogRead(ANALOG3);
  // #else
  dL = fit(analogRead(ANALOG1));
  dR = fit(analogRead(ANALOG2));
  // #endif

  meanD = (dL + dR) / 2;
  maxD = max(dL, dR);
  minD = min(dL, dR);
  if (0)
  {
    // PT("rL ");
    // PT(rawL);
    // PT("\trR ");
    // PT(rawR);
    PT("\tdL ");
    PT(dL);
    PT("\tdR ");
    PT(dR);
    PT("\tmD ");
    PT(meanD);
  }
}

void catchObject()
{
  while (dL < 30 || dR < 30)
  {
    readDistancePins();
    distancePID(dL, dR);
    if (dL < 10) // within capture range
      leftCloseTimer = millis();
    if (dR < 10) // within capture range
      rightCloseTimer = millis();
    // if (currentAng[2] < 20)
    calibratedPWM(2, min(float(80), 80 - (min(dL, dR) - 15) * 5)); // open pincer
    if (leftCloseTimer > 0 && leftCloseTimer > 0 && abs(leftCloseTimer - rightCloseTimer) < 50)
    {
      int8_t targetShift = (leftCloseTimer - rightCloseTimer); // evaluate the target's speed and direction
      PTHL("targetShift", targetShift);
      calibratedPWM(0, currentAng[0] + targetShift);
      calibratedPWM(1, 45);  // raise arm
      calibratedPWM(2, 120); // clip the pincer
      delay(1000);
      tQueue->addTask(T_INDEXED_SIMULTANEOUS_ASC, "0,0,1,0,2,0", 1000);
      leftCloseTimer = rightCloseTimer = -1;
    }

  }
}

void read_doubleInfraredDistance()
{
  readDistancePins();
  // if (makeSound && minD > longThres / 2 && maxD < longThres && periodGlobal == 1)
  //   beep(35 - meanD, meanD / 4);
#ifdef NEOPIXEL_PIN
  strip.clear();
  for (int i = 0; i < min(8 - sqrt(dL) * 1.4, strip.numPixels()); i++)
  {                                                                                              // For each pixel in strip..
    strip.setPixelColor(i, strip.Color(255 - meanD * 6, meanD * 6, 128 + currentXDistance * 2)); //  Set pixel's color (in RAM)
    strip.show();
  }
#endif
#ifdef LED_PIN
  if (LED_PIN == 10)
    digitalWrite(LED_PIN, 255 - meanD * 6 > 128);
  else
    analogWrite(LED_PIN, 255 - meanD * 6);
#endif
  if (dL < 1 || dR < 1)
  {
    readDistancePins();
    if (dL < 1 && dR < 1)
    {
      // makeSound = !makeSound;
      // tQueue->addTask('k', "bk", 1500);
      // tQueue->addTask('k', "wkF", 1500);
      // tQueue->addTask('k', "sit");
      // tQueue->addTask('i', "");
    }
  }
#ifdef ROBOT_ARM

#endif
  if (periodGlobal == 1)
  {
    distancePID(dL, dR);
    #ifdef DEBUG_IR_DISTANCE
    PT("dL:"); PT(dL); PT(" dR:"); PT(dR); PT(" avg:"); PT(meanD);
    PT(" BG:"); PT(backgroundThreshold()); PT(" near:"); PT(NEAR_SINGLE_DISTANCE); PTL();
    #endif
    // 根据模式决定是否在静止状态触发肢体动作
    if (irBehaviorMode == IR_SIT_TRACK)
    {
      // 仅坐下追踪：不触发行走类动作
    }
    else
    {
      // 行走避障模式：保留之前在静止状态的必要转向/后退，以起步准备
      if (tQueue->cleared())
      {
        if (dL < longThres / 5 || dR < longThres / 5)
        {
          tQueue->addTask('k', dL < dR ? "vtR" : "vtL", 2000);
        }
        else
        {
          if (abs(dR - dL) > 2)
          {
            tQueue->addTask('k', dL < dR ? "trR" : "trL", 1000);
          }
          else if (dL < longThres / 2 || dR < longThres / 2)
          {
            tQueue->addTask('k', dL < dR ? "bkL" : "bkR", 1500);
            tQueue->addTask('k', dL < dR ? "trR" : "trL", 1500);
          }
        }
      }
    }
    // if (currentXDistance < -75 || currentXDistance > 75) {
    //   if (currentXDistance < -75) {
    //     // tQueue->addTask('k', "vtR", 2000);
    //   } else {
    //     // tQueue->addTask('k', "vtL", 2000);
    //   }
    //   // tQueue->addTask('k', "sit");
    //   currentXDistance = 0;
    //   resetPID();
    // }
  }
  // distanceNaive(dL, dR);
  else if (periodGlobal > 1 && tQueue->cleared())
  { // gait
    if (irBehaviorMode == IR_SIT_TRACK)
    {
      // 坐下追踪模式：不注入任何行走相关指令，完全静默
    }
    else if (dL > longThres && dR > longThres)
    { // free to run
      tQueue->addTask('i', "");
      tQueue->addTask('k', "trF");
      idleTimer = millis() + IDLE_TIME / 2;
      PTLF(" free");
    }
    else if (dL < longThres / 5 || dR < longThres / 5)
    { // too close. retreat
      tQueue->addTask('i', "");
      tQueue->addTask('k', dL < dR ? "vtR" : "vtL", 2000);
      PTLF(" too close");
    }
    else
    {
      idleTimer = millis() + IDLE_TIME;
      if (abs(dR - dL) > 2)
      { // one side has longer free distance
        tQueue->addTask('i', "");
        tQueue->addTask('k', dL < dR ? "trR" : "trL", 1000);
        PTLF("turn");
      }
      else if (dL < longThres / 2 || dR < longThres / 2)
      {
        tQueue->addTask('i', "");
        tQueue->addTask('k', dL < dR ? "bkL" : "bkR", 1500);
        tQueue->addTask('k', dL < dR ? "trR" : "trL", 1500);
        PTLF(" retreat and turn");
      }
    }
  }
  // PTL();
}
