# BuddyBot Safety Test Plan

This document outlines the standard verification steps required to validate the safety systems of the BuddyBot firmware. It must be run before any major release, and especially after touching `RobotHal.cpp`, `ServoDrive.cpp`, `WifiControl.cpp`, or `AutonomyManager.cpp`.

## Prerequisites
- Hardware: M5StickS3 connected to Unit 8Servos, 4 continuous rotation servos connected on ports 0-3.
- Firmware flashed with `ALLOW_MOTOR_ARMING = true` (or the safe default modified temporarily for testing).
- Sonic sensor connected to I2C.

## 1. Boot Safety
1. **Power on** BuddyBot.
2. Verify the screen displays "BuddyBot OS", runs boot diagnostics, and the face appears.
3. Verify motors are **NOT spinning**.
4. Check the serial console for `BOOT: -> COMPLETE`.
5. Run the `STATUS` command. Verify `motors_armed=false`, `drive_mode=STOPPED`, and `safety_state=disarmed`.

## 2. Obstacle Prevention (Sonic)
1. Arm motors:
   - Connect via Web UI and click `ARM`.
   - OR send `ARM` command via Serial.
2. Verify motors emit a short PROUD expression and `STATUS` shows `motors_armed=true`.
3. Command forward drive: `MOVE FORWARD 2000` via Serial or hold the Web UI Forward button.
4. While driving forward, place an obstacle < 120mm in front of the Sonic sensor.
5. **Expected Result**: 
   - Motors stop immediately, but the robot remains armed for a bounded reverse/turn recovery.
   - Expression changes to WORRIED/SCARED.
   - `STATUS` shows `last_stop_reason=obstacle_blocked`; `safety_state` remains `armed` unless another fault is present.
6. Command forward drive again while the obstacle is still present.
7. **Expected Result**: 
   - Motors do not spin.
   - System logs or returns an `obstacle_blocked` warning.
8. Verify a reverse or turn command remains available for recovery, then clear the obstacle before commanding forward motion.

## 3. Sensor Failure and Watchdog
1. While armed, unplug the Sonic sensor from the I2C port.
2. Command forward drive.
3. **Expected Result**:
   - Forward drive is denied, the robot disarms, and recovery requires a healthy sensor plus an explicit arm.
   - `STATUS` shows `obstacle_safety=sensor_unavailable`, `safety_state=fault`, or `safety_fault=range_sensor_invalid`.

## 4. Web UI Disconnect / Keepalive
1. Refresh the Web UI while actively driving forward (simulating a disconnect).
2. **Expected Result**:
   - The robot stops within 350ms (WIFI_DRIVE_WATCHDOG_MS).
   - The robot is disarmed and `safety_state=fault`.
   - `safety_fault` is `drive_watchdog`, `controller_disconnected`, or `controller_lease_expired`.

## 5. Firmware Lockdown
1. Change `ALLOW_MOTOR_ARMING` to `false` in `src/Config.h`.
2. Recompile and flash.
3. Attempt to arm motors via Web UI or Serial `ARM` command.
4. **Expected Result**:
   - Motors do NOT arm.
   - BuddyBot plays a WORRIED expression.
   - `STATUS` shows `motors_armed=false`.

## 6. Local E-Stop and Recovery
1. Arm motors on an elevated test rig.
2. Hold both device buttons for at least `PHYSICAL_ESTOP_HOLD_MS`.
3. **Expected Result**:
   - Drive output stops immediately and remains disarmed.
   - Active manipulator motion is cancelled without commanding a new rest pose.
   - `STATUS` reports `safety_state=estop` and `safety_fault=physical_estop`.
4. Verify serial and Wi-Fi arm/move requests are denied.
5. Hold both buttons for `PHYSICAL_ESTOP_RESET_HOLD_MS`.
6. **Expected Result**:
   - `STATUS` reports `safety_state=disarmed`.
   - Movement remains disabled until an explicit arm request.

## 7. Autonomy IMU Guard
1. With a clear range reading, arm the robot and enable autonomy.
2. Disconnect or disable the IMU, or place the test rig beyond its configured tilt limit.
3. **Expected Result**:
   - Autonomy stops and is disabled.
   - The robot disarms with an IMU safety fault.
   - Restoring the IMU does not resume motion; explicit re-arm and autonomy enablement are required.

## 8. Action Motion Routing
1. Temporarily enable `ALLOW_ACTION_DRIVE_MOVEMENT` only on an elevated test rig.
2. Start `ACTION DANCE` with a clear range reading, then place an obstacle within 120mm before its forward step.
3. **Expected Result**: the forward step is rejected by the same obstacle policy as a manual move; the action does not bypass safety routing.
