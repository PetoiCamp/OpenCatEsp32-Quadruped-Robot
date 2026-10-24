#pragma once
// Play one behavior's head keyframes on top of the current gait or posture.
// Behavior frames start after the 7-byte header. The first HEAD_GROUP_LEN
// angles are the head, and the bytes at DOF and DOF+1 are speed and delay.
// Timing matches transform(): cosine interpolation, speed/8 degrees per step,
// one step every DOF/2 ms, then hold abs(delay)*50 ms. The sequence runs once.

class HeadOverlay {
public:
  bool active;
  float angles[HEAD_GROUP_LEN];

  HeadOverlay() {
    active = false;
    retire = false;
    frameCount = 0;
    frameIndex = 0;
    steps = 0;
    stepMs = DOF / 2;
    if (stepMs < 1)
      stepMs = 1;
    moveMs = 0;
    holdMs = 0;
    frameStartMs = 0;
    source = NULL;
    mirrorHead = false;
    for (byte i = 0; i < HEAD_GROUP_LEN; i++) {
      angles[i] = 0;
      fromAng[i] = 0;
      targetAng[i] = 0;
    }
  }

  void stop() {
    active = false;
    retire = false;
  }

  void start(const int8_t *progmem, bool mirrorHeadQ = false) {
    source = progmem;
    mirrorHead = mirrorHeadQ;
    frameCount = abs((int)(int8_t)pgm_read_byte(source));
    angleDataRatio = (int8_t)pgm_read_byte(source + 3);
    if (angleDataRatio < 1)
      angleDataRatio = 1;
    frameSize = DOF + 4;
    retire = false;
    active = frameCount > 0;
    for (byte i = 0; i < HEAD_GROUP_LEN; i++)
      fromAng[i] = currentAng[i] - currentAdjust[i];
    if (active)
      beginFrame(0);
  }

  // Advance the head pose. Call once per gait/posture frame, before servos are written.
  void update() {
    if (!active)
      return;
    if (retire) {
      active = false;
      retire = false;
      return;
    }
    unsigned long elapsed = millis() - frameStartMs;
    if (elapsed < moveMs && steps > 0) {
      float s = (float)elapsed / (float)stepMs;
      if (s > steps)
        s = steps;
      float blend = (1.0f + cosf(M_PI * s / steps)) / 2.0f;  // 1 at the start, 0 at the target
      for (byte i = 0; i < HEAD_GROUP_LEN; i++)
        angles[i] = targetAng[i] + blend * (fromAng[i] - targetAng[i]);
      return;
    }
    for (byte i = 0; i < HEAD_GROUP_LEN; i++)
      angles[i] = targetAng[i];
    if (elapsed < moveMs + holdMs)
      return;
    for (byte i = 0; i < HEAD_GROUP_LEN; i++)
      fromAng[i] = targetAng[i];
    if (frameIndex + 1 >= frameCount) {
      retire = true;  // keep this pose for the current servo write, then release the head
      return;
    }
    beginFrame(frameIndex + 1);
    for (byte i = 0; i < HEAD_GROUP_LEN; i++)
      angles[i] = fromAng[i];
  }

private:
  const int8_t *source;
  int frameCount;
  int frameIndex;
  int frameSize;
  int steps;
  int8_t angleDataRatio;
  unsigned long stepMs;
  unsigned long moveMs;
  unsigned long holdMs;
  unsigned long frameStartMs;
  bool retire;
  bool mirrorHead;
  float fromAng[HEAD_GROUP_LEN];
  float targetAng[HEAD_GROUP_LEN];

  void beginFrame(int index) {
    frameIndex = index;
    const int8_t *framePtr = source + 7 + index * frameSize;
    int maxDiff = 0;
    for (byte i = 0; i < HEAD_GROUP_LEN; i++) {
      targetAng[i] = (int8_t)pgm_read_byte(framePtr + i) * angleDataRatio;
      if (mirrorHead && (i == 0
#ifndef ROBOT_ARM
                         || i == 2
#endif
                         ))
        targetAng[i] = -targetAng[i];
      maxDiff = max(maxDiff, abs((int)(fromAng[i] - targetAng[i])));
    }
    int8_t speed = (int8_t)pgm_read_byte(framePtr + DOF);
    int8_t delayByte = (int8_t)pgm_read_byte(framePtr + DOF + 1);
    float speedRatio = speed / 8.0f;
    steps = speedRatio > 0 ? (int)round(maxDiff / speedRatio) : 0;
    moveMs = (unsigned long)steps * stepMs;
    holdMs = stepMs + (unsigned long)abs((int)delayByte) * 50UL;
    frameStartMs = millis();
  }
};

HeadOverlay headOverlay;
