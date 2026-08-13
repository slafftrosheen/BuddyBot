# Servo Wiring and Calibration Guide

This document describes the 4-Wheel Drive + Manipulator configuration for Halo BuddyBot using a single M5Stack Unit 8Servos module.

## Channel Map (SERVO8_FOUR_WHEEL_MANIPULATOR)

| Channel | Role | Type | Rest/Stop | Expected Direction |
|---------|------|------|-----------|--------------------|
| 0 | Front-Left Wheel | Continuous | 1500us | Forward |
| 1 | Rear-Left Wheel | Continuous | 1500us | Forward |
| 2 | Front-Right Wheel| Continuous | 1500us | Inverted |
| 3 | Rear-Right Wheel | Continuous | 1500us | Inverted |
| 4 | Head | Positional | 90 deg | Normal |
| 5 | Left Arm | Positional | 90 deg | Normal |
| 6 | Right Arm | Positional | 90 deg | Normal |
| 7 | Accessory 1 | Positional | 90 deg | Normal |

## Wiring Instructions

1. Ensure the Unit 8Servos module has external power connected before plugging in any servos. The M5StickS3 cannot provide enough current over I2C.
2. Wire the left-side drive servos to channels 0 and 1.
3. Wire the right-side drive servos to channels 2 and 3.
4. Wire the head, left arm, right arm, and accessory to channels 4, 5, 6, 7 respectively.
5. All continuous servos MUST be mechanically trimmable or properly calibrated in software, see below.

## Calibration

By default, Halo BuddyBot disables `ALLOW_MOTOR_ARMING` and `ALLOW_ACTION_DRIVE_MOVEMENT`.

1. To calibrate wheels, elevate the chassis so the wheels can spin freely.
2. Enable `ALLOW_MOTOR_ARMING` in `src/Config.h`.
3. Use the `SystemStatus` output via the Serial Monitor to observe the `drive_armed` state.
4. If wheels rotate while `DriveMode::STOPPED`, adjust the `stopUs` value for that channel in `DEFAULT_4WD_SERVO_CONFIG` (located in `src/BuildProfiles.cpp`) until it completely stops.
5. Once stops are calibrated, test `DriveMode::FORWARD` and `DriveMode::REVERSE` to ensure wheels rotate the correct direction. Invert the `direction` field if necessary.
6. Verify the arm and head angles are physically safe.
7. Only after this dry run should you place the robot on the floor.
