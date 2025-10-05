# Gait Control Implementation Guide

## Overview

This document describes the enhanced gait control functionality implemented in the OpenCat ESP32 firmware. The system now supports more precise control over gait execution by allowing arguments to be passed with gait commands.

## Features

### 1. **Cycle-Based and Time-Based Straight Gaits**
Execute a straight gait with precise control using either cycle count or time duration:

**Cycle-Based (parameter < 200)**:
```
k wkF 5       # Walk forward for 5 gait cycles
k bkF 10      # Walk backward for 10 gait cycles
```

**Time-Based (parameter ≥ 200)**:
```
k wkF 5000    # Walk forward for 5000ms (5 seconds)
k bkF 3000    # Walk backward for 3000ms (3 seconds)
```

### 2. **Angle-Controlled Turning Gaits**
Execute a turning gait until a specific angle is reached:
```
k wkL 90      # Turn left (clockwise) 90 degrees
k wkR 45      # Turn right (counterclockwise) 45 degrees
```

### 3. **Polar Coordinate System**
The system uses polar coordinate convention where:
- Positive angles are counterclockwise (right turn)
- Negative angles are clockwise (left turn)
- This matches standard mathematical conventions

## Technical Implementation

### 1. Yaw Coordinate System Correction

**Problem**: Original ypr[0] (yaw angle) direction was inconsistent with polar coordinate definition:
- Original: positive angle = clockwise direction
- Expected: positive angle = counterclockwise direction (polar coordinate convention)

**Solution**: Negate ypr[0] at the IMU data source rather than at usage time.

**Modifications**:
- `src/imu.h - readIMU()` function: Add `ypr[0] = -ypr[0];` for both ICM42670 and MPU6050
- `src/imu.h - print6Axis()` function: Use `-icm.ypr[0]` and `-mpu.ypr[0]` for display
- Remove negation operations in usage locations

### 2. Turning Control Variables

```cpp
// In src/imu.h
bool turningQ = false;           // Turning control switch
float targetYawAngle = 0.0;     // Target angle to reach
float initialYawAngle = 0.0;    // Initial angle when turning started
bool needTurning = false;        // Flag to prevent turning exception from being skipped
```

### 3. Cycle Counting Implementation

**Global Variables** (in `src/OpenCat.h`):
```cpp
int targetCycles = 0;           // Target number of gait cycles to complete
int completedCycles = 0;        // Number of gait cycles completed so far
bool cycleCountingMode = false; // Whether cycle counting mode is active
```

**Cycle Counting Logic** (in `src/skill.h - Skill::perform()`):
```cpp
frame += tStep;
if (frame >= abs(period)) {
  frame = 0;  // One gait cycle completed
  
  // Check if in cycle counting mode
  if (cycleCountingMode && period > 1) {
    completedCycles++;
    
    if (completedCycles >= targetCycles) {
      // Target reached, stop the gait
      cycleCountingMode = false;
      completedCycles = 0;
      targetCycles = 0;
      tQueue->addTask('k', "up");
    }
  }
}
```

**Mode Selection** (in `src/reaction.h - T_SKILL case`):
```cpp
if (timeOrAngle < 200) {
  // Cycle-based control
  cycleCountingMode = true;
  targetCycles = timeOrAngle;
  completedCycles = 0;
} else {
  // Time-based control
  cycleCountingMode = false;
  tQueue->addTask('k', skillName, timeOrAngle);
  tQueue->addTask('k', "up");
}
```

### 4. Enhanced T_SKILL Command Processing

**Argument Parsing**:
- Parse skill commands with space-separated arguments
- Extract skill name and timing/angle parameter
- Support both formats: `k wkF 5000` or `k wkL 90`

**Straight Gaits (F suffix)**:
- Parameter interpretation:
  - If parameter < 200: Treat as number of gait cycles
    - Enable cycle counting mode (`cycleCountingMode = true`)
    - Set target cycle count (`targetCycles`)
    - Monitor `frame` variable: when it returns to 0, increment `completedCycles`
    - Stop gait when `completedCycles >= targetCycles`
    - Provides precise control based on actual gait cycle completion
  - If parameter ≥ 200: Treat as milliseconds directly
    - Uses task queue with timing for time-based control
- Execute gait until target is reached (cycles or time)

**Turning Gaits (L/R suffix)**:
- Set up turning control parameters
- Record initial yaw angle
- Calculate target angle based on direction and offset
- Normalize angles to -180 to 180 degree range

### 5. IMU Exception Handling

**New Exception Type**:
```cpp
#define IMU_EXCEPTION_TURNING -7
```

**Detection Logic**:
- Monitor current yaw difference from initial angle
- Check if target angle is reached or exceeded
- Special handling for ±180 degree boundary cases:
  - When `targetDiff ≈ ±180°`, use proximity check instead of >= or <= comparison
  - Prevents issue where 180° and -180° are the same point but fail comparison
- Set `imuException = IMU_EXCEPTION_TURNING` when target reached
- Disable turning control: `turningQ = false`

**Exception Processing**:
- Handle `IMU_EXCEPTION_TURNING` in `dealWithExceptions()`
- Add "up" task to stop robot and make it stand
- Print debug information

## Serial Command Usage

### Command Format

The enhanced T_SKILL command supports arguments for precise gait control:

```
k <skill_name> <argument>
```

Where:
- `k` is the T_SKILL token
- `<skill_name>` is the gait skill name (e.g., wkF, wkL, wkR, bkF)
- `<argument>` is the timing (for straight gaits) or angle (for turning gaits)

### Straight Gait Commands

For straight gaits (skills ending with 'F'), the argument can be either **cycle count** or **milliseconds**:

#### Cycle-Based Control (parameter < 200)
When the parameter is less than 200, it represents the number of gait cycles to execute:

```
k wkF 5       # Walk forward for 5 gait cycles
k bkF 10      # Walk backward for 10 gait cycles
k vtF 3       # Trot forward for 3 gait cycles
```

**Understanding Gait Cycles**:
- Each gait has a `period` which is the number of frames in one complete gait cycle
- Each time `frame` returns to 0, one gait cycle is completed
- The system directly counts completed cycles in real-time, no time conversion needed

#### Time-Based Control (parameter ≥ 200)
When the parameter is 200 or greater, it represents execution time in milliseconds:

```
k wkF 5000    # Walk forward for 5000ms (5 seconds)
k bkF 3000    # Walk backward for 3000ms (3 seconds)
k vtF 2000    # Trot forward for 2000ms (2 seconds)
```

**Logic**:
1. Parse skill name and timing/cycle argument
2. If argument < 200: Enable cycle counting mode
   - Set `targetCycles` = argument value
   - Set `completedCycles` = 0
   - Set `cycleCountingMode` = true
   - Each time `frame` returns to 0, increment `completedCycles`
   - When `completedCycles >= targetCycles`, add "up" task to stop
3. If argument ≥ 200: Use time-based mode
   - Add task to queue: `tQueue->addTask('k', "wkF", timeInMs)`
   - Task queue executes the gait for specified duration
   - Robot automatically stops after duration expires

### Turning Gait Commands

For turning gaits (skills ending with 'L' or 'R'), the argument represents target angle in degrees:

```
k wkL 90      # Turn left (clockwise) 90 degrees
k wkR 45      # Turn right (counterclockwise) 45 degrees
k vtL 180     # Trot left 180 degrees
k vtR 30      # Trot right 30 degrees
```

**Logic**:
1. Parse skill name and angle argument
2. Set up turning control:
   - `turningQ = true`
   - `initialYawAngle = ypr[0]` (current yaw)
   - `targetYawAngle = initialYawAngle ± angle` (based on direction)
3. IMU continuously monitors yaw angle
4. When target reached: `imuException = IMU_EXCEPTION_TURNING`
5. Exception handler adds "up" task: `tQueue->addTask('k', "up")`
6. Robot stops and stands up

### Coordinate System

The system uses polar coordinate convention:
- **Positive angles** = Counterclockwise (right turn)
- **Negative angles** = Clockwise (left turn)
- **wkR** = Turn right (counterclockwise, positive angle)
- **wkL** = Turn left (clockwise, negative angle)

### Task Queue Integration

You can combine these commands with the task queue system for complex sequences:

```
# Basic sequence: walk forward, turn left, walk forward again
qk wkF 2000:k wkL 90:k wkF 2000:

# Complex sequence with different gaits
qk wkF 3000:k vtL 45:k bkF 1000:k vtR 90:k up:1000:

# Mixed commands with delays
qk sit:1000:k wkF 2000:k wkR 180:k up:500:
```

**Task Queue Format**:
```
q<token><command>:<delay>><token><command>:<delay>...
```

### Backward Compatibility

Commands without arguments work exactly as before:

```
k sit        # Sit posture (no timing/angle)
k up         # Stand up posture
k wkF        # Walk forward (continuous, no timing)
k wkL        # Walk left (continuous, no angle control)
```

### Debug Output

The system provides detailed debug information:

**When starting cycle-based gait**:
```
Cycle counting mode: 5 cycles, Period: 16
```

**During cycle execution**:
```
Completed cycle: 1 / 5
Completed cycle: 2 / 5
Completed cycle: 3 / 5
Completed cycle: 4 / 5
Completed cycle: 5 / 5
Cycle target reached, stopping gait
```

**When starting time-based gait**:
```
Time-based mode: wkF for 5000 ms
```

**When starting turning gait**:
```
Started turning gait: wkL
initial yaw: 45.2
target angle: 135.2
turning direction: LEFT (CW)
```

**When target is reached**:
```
Turning target reached! Current yaw: 135.1
Target was: 135.2
EXCEPTION: turning target reached
endTurn
```

### Error Handling

- **Invalid arguments**: Commands with invalid numbers are ignored
- **Missing arguments**: Commands without arguments work as before
- **Angle normalization**: Angles are automatically normalized to -180 to 180 degrees
- **180° boundary handling**: Special logic handles the ±180° boundary case where both values represent the same angle
- **Exception protection**: Turning exceptions are protected from being skipped

### Examples

#### Basic Movement Patterns
```
# Square pattern (time-based)
k wkF 2000    # Forward 2s
k wkR 90      # Turn right 90°
k wkF 2000    # Forward 2s
k wkR 90      # Turn right 90°
k wkF 2000    # Forward 2s
k wkR 90      # Turn right 90°
k wkF 2000    # Forward 2s
k wkR 90      # Turn right 90°

# Square pattern (cycle-based)
k wkF 10      # Forward 10 cycles
k wkR 90      # Turn right 90°
k wkF 10      # Forward 10 cycles
k wkR 90      # Turn right 90°
k wkF 10      # Forward 10 cycles
k wkR 90      # Turn right 90°
k wkF 10      # Forward 10 cycles
k wkR 90      # Turn right 90°

# Circle pattern (mixed control)
k wkF 5       # Forward 5 cycles
k wkL 30      # Turn left 30°
k wkF 5       # Forward 5 cycles
k wkL 30      # Turn left 30°
# ... repeat for full circle

# Precise short movements (cycle-based)
k wkF 1       # Forward exactly 1 cycle
k wkR 45      # Turn right 45°
k wkF 2       # Forward exactly 2 cycles
k wkL 90      # Turn left 90°
```

#### Advanced Sequences
```
# Patrol pattern with task queue
qk wkF 3000:k vtL 90:k wkF 2000:k vtR 90:k wkF 3000:k vtL 90:k wkF 2000:k vtR 90:

# Search pattern
qk wkF 1000:k wkL 45:k wkF 1000:k wkL 45:k wkF 1000:k wkL 45:k wkF 1000:k wkL 45:
```

## Implementation Flow

### Straight Gait Flow

**Cycle-Based (parameter < 200)**:
1. User sends command: `k wkF 5`
2. System parses: skill="wkF", parameter=5
3. System checks: parameter (5) < 200, treat as cycles
4. System enables cycle counting:
   - `cycleCountingMode = true`
   - `targetCycles = 5`
   - `completedCycles = 0`
5. During gait execution (in `skill->perform()`):
   - Each frame is executed normally
   - When `frame >= period`, `frame` resets to 0
   - At this moment, `completedCycles` is incremented
   - When `completedCycles >= 5`, system adds "up" task to stop
6. Robot stops precisely after completing 5 full gait cycles

**Time-Based (parameter ≥ 200)**:
1. User sends command: `k wkF 5000`
2. System parses: skill="wkF", parameter=5000
3. System checks: parameter (5000) ≥ 200, treat as milliseconds
4. Adds task to queue: `tQueue->addTask('k', "wkF", 5000)`
5. Task queue executes gait for specified duration

### Turning Gait Flow
1. User sends command: `k wkL 90`
2. System parses: skill="wkL", angle=90
3. Sets up turning control:
   - `turningQ = true`
   - `initialYawAngle = ypr[0]`
   - `targetYawAngle = initialYawAngle + 90`
4. IMU continuously monitors yaw angle
5. When target reached: `imuException = IMU_EXCEPTION_TURNING`
6. Exception handler adds "up" task: `tQueue->addTask('k', "up")`
7. Robot stops and stands up

## Debug Output

The system provides comprehensive debug output:

**When starting turning gait**:
```
Started turning gait: wkL
initial yaw: 45.2
target angle: 135.2
turning direction: LEFT (CW)
```

**When target is reached**:
```
Turning target reached! Current yaw: 135.1
Target was: 135.2
EXCEPTION: turning target reached
endTurn
```

## Key Features

1. **Dual Control Modes** - Support both cycle-based and time-based control for straight gaits
2. **Automatic Unit Detection** - Parameters < 200 treated as cycles, ≥ 200 as milliseconds
3. **Real-Time Cycle Counting** - Direct counting of completed gait cycles, no time conversion
4. **Precise Movement Control** - Cycle-based control allows exact gait repetitions
5. **Consistency** - Global ypr[0] values are correct throughout the system
6. **Simplicity** - No complex atomic operations or cross-core synchronization
7. **Maintainability** - Changes made at data source level
8. **Debug Friendly** - print6Axis() and other outputs show correct values
9. **Reliability** - Direct exception handling without complex flag management
10. **Backward Compatibility** - Commands without arguments work exactly as before
11. **Task Queue Integration** - Uses existing task queue mechanism for execution

## Notes

### Straight Gaits (F suffix)
- **Parameter < 200**: Treated as number of gait cycles
  - Provides precise control based on complete gait cycles
  - System directly counts cycles in real-time by monitoring `frame` variable
  - When `frame` returns to 0, one cycle is completed and counter increments
  - Stops exactly when target cycle count is reached
  - Useful for exact repetitions (e.g., "walk exactly 5 steps")
- **Parameter ≥ 200**: Treated as milliseconds
  - Uses task queue with timing for time-based control
  - Better for timed movements (e.g., "walk for 3 seconds")

### Turning Gaits (L/R suffix)
- All angles are normalized to the -180 to 180 degree range
- The system automatically handles angle wrapping (e.g., turning from 170° to -170°)
- **Special 180° handling**: Since 180° and -180° represent the same angle, turns near ±180° use proximity checking (within 5°) instead of exact comparison
- Turning control is automatically disabled when the target is reached
- The robot will stop and stand up ("up" command) when the turning target is reached
- The system uses polar coordinate convention: positive angles are counterclockwise (right turn), negative angles are clockwise (left turn)
- No complex cross-core communication ensures turning exceptions are handled reliably

## 180 Degree Boundary Fix

### Problem Description

When setting a turning gait to exactly 180 degrees, the robot would not stop even after reaching the target angle.

**Root Cause**:
- In a circular angle system, 180° and -180° represent the same physical angle
- The original comparison logic only checked `targetDiff > 0` or `targetDiff < 0`
- When `targetDiff` equals exactly ±180°, neither condition is satisfied
- Result: The target detection fails and the robot keeps turning

**Example Scenario**:
```
Initial angle: 0°
Target: turn 180°
Target angle: 0 + 180 = 180°

targetDiff = 180 - 0 = 180
After normalization: targetDiff = 180 (boundary value)

Original check:
if (targetDiff > 0 && currentYawDiff >= targetDiff)  // 180 > 0 is true, but...
   // Due to floating point and boundary effects, this might fail

The condition might not trigger properly at the ±180° boundary.
```

### Solution

Added special handling for angles near ±180°:

```cpp
bool targetReached = false;
if (fabs(targetDiff) >= 179.5) {
  // For ~180 degree turns, use proximity check
  targetReached = fabs(currentYawDiff - targetDiff) < 5.0 || 
                  fabs(fabs(currentYawDiff) - 180) < 5.0;
} else if (targetDiff > 0) {
  targetReached = currentYawDiff >= targetDiff;
} else if (targetDiff < 0) {
  targetReached = currentYawDiff <= targetDiff;
}
```

**Key Changes**:
1. Detect when `targetDiff` is near ±180° (threshold: 179.5°)
2. Use proximity checking (within 5°) instead of exact comparison
3. Check both the target difference and the ±180° boundary
4. This ensures the turn stops even at the boundary case

**Benefits**:
- 180° turns now work correctly
- 179°, 181° and other near-boundary angles also work reliably
- No false positives due to the 5° tolerance zone

## Turning Exception Fix

### Problem Description

The turning exception was being skipped by the main program due to a race condition:

1. **Exception Generation**: When the turning target is reached, `getImuException()` sets `imuException = IMU_EXCEPTION_TURNING`
2. **Main Loop Timing**: The main program may have already passed `dealWithExceptions()` when the exception is generated
3. **Exception Reset**: In the next loop iteration, `getImuException()` resets `imuException = 0` before `dealWithExceptions()` can process it

### Solution

Added a `needTurning` flag to prevent the exception from being reset before it can be processed.

#### Implementation

1. **New Variable**:
   ```cpp
   bool needTurning = false;  // Flag to prevent turning exception from being skipped
   ```

2. **Exception Generation** (in `getImuException()`):
   ```cpp
   if (target_reached) {
     imuException = IMU_EXCEPTION_TURNING;
     turningQ = false;
     needTurning = true;  // Set flag to prevent exception from being skipped
   }
   ```

3. **Exception Protection** (in `getImuException()`):
   ```cpp
   } else if (!needTurning)  // Only reset exception if not waiting for turning processing
     imuException = 0;
   ```

4. **Exception Processing** (in `dealWithExceptions()`):
   ```cpp
   case IMU_EXCEPTION_TURNING:
     tQueue->addTask('k', "up");
     needTurning = false;  // Reset flag after creating task
     break;
   ```

#### Flow

1. **Turning Target Reached**: `needTurning = true`, `imuException = IMU_EXCEPTION_TURNING`
2. **Exception Protected**: `getImuException()` won't reset the exception while `needTurning = true`
3. **Exception Processed**: `dealWithExceptions()` creates the "up" task
4. **Flag Reset**: `needTurning = false` allows normal exception handling to resume

#### Benefits

- **Reliable Exception Handling**: Turning exceptions are never missed
- **Simple Implementation**: Minimal code changes
- **No Complex Synchronization**: Uses simple boolean flag
- **Maintains Existing Logic**: All other exception handling remains unchanged 
