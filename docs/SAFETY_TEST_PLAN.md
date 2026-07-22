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
5. Run the `STATUS` command. Verify `motors_armed=false` and `drive_mode=STOPPED`.

## 2. Obstacle Prevention (Sonic)
1. Arm motors:
   - Connect via Web UI and click `ARM`.
   - OR send `ARM` command via Serial.
2. Verify motors emit a short PROUD expression and `STATUS` shows `motors_armed=true`.
3. Command forward drive: `MOVE FORWARD 2000` via Serial or hold the Web UI Forward button.
4. While driving forward, place an obstacle < 120mm in front of the Sonic sensor.
5. **Expected Result**: 
   - Motors stop immediately.
   - Expression changes to WORRIED/SCARED.
   - `STATUS` shows `obstacle_safety=blocked` and `last_stop_reason=obstacle_blocked`.
6. Command forward drive again while the obstacle is still present.
7. **Expected Result**: 
   - Motors do not spin.
   - System logs or returns an `obstacle_blocked` warning.
8. Command reverse drive.
9. **Expected Result**: 
   - Motors spin backwards (reverse motion is permitted even when blocked in front).

## 3. Sensor Failure and Watchdog
1. While armed, unplug the Sonic sensor from the I2C port.
2. Command forward drive.
3. **Expected Result**:
   - Drive is denied.
   - `STATUS` shows `obstacle_safety=sensor_unavailable` or `last_stop_reason=range_sensor_invalid`.

## 4. Web UI Disconnect / Keepalive
1. Refresh the Web UI while actively driving forward (simulating a disconnect).
2. **Expected Result**:
   - The robot stops within 350ms (WIFI_DRIVE_WATCHDOG_MS).
   - `last_stop_reason` is `drive_watchdog` or `manual_stop`.

## 5. Firmware Lockdown
1. Change `ALLOW_MOTOR_ARMING` to `false` in `src/Config.h`.
2. Recompile and flash.
3. Attempt to arm motors via Web UI or Serial `ARM` command.
4. **Expected Result**:
   - Motors do NOT arm.
   - BuddyBot plays a WORRIED expression.
   - `STATUS` shows `motors_armed=false`.
