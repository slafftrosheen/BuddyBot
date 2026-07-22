# Release Baseline: BuddyBot 0.3.0-beta.1

This document captures the baseline configuration for the `hardware-safety-beta` channel of BuddyBot.

## Release Identity
- **Name:** BuddyBot OS
- **Version:** 0.3.0-beta.1
- **Channel:** hardware-safety-beta

## Core Features Included
- Modular Hardware Abstraction Layer (HAL).
- Serial Protocol and Local Wi-Fi SoftAP Control (WebSocket).
- Pairing / Controller / Observer Access Model.
- Unit 8Servos 4WD Drive + 4 Positional Manipulator Channels.
- Sonic Range Sensor Safety and Assisted Autonomy.
- Expression Engine and Visual Action Cues on M5StickS3 display.

## Exclusions
This release intentionally does **not** include:
- Cloud control or Internet connectivity.
- OTA endpoints.
- BLE.
- Camera or Vision processing.
- Speech recognition.
- Unrestricted Autonomy (roaming is strictly safe-mode assisted avoidance).
- Browser raw servo joint control (all commands flow through API logic).

## Safety Status
By default, the firmware compiles in a **Hardware-Safe Mode**. 
- `ALLOW_MOTOR_ARMING = false`
- `ENABLE_AUTONOMY_AT_BOOT = false`
- `ENABLE_CAUTIOUS_ROAM = false`
- `ALLOW_ACTION_DRIVE_MOVEMENT = false`

The operator must perform a physical bench test and local calibration before enabling motor arming. Refer to `SAFETY_TEST_PLAN.md` for the correct procedures.
