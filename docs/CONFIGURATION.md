# Configuration Guide

Halo BuddyBot uses a unified configuration file (`src/Config.h`) to manage everything from hardware settings to safety defaults.
Config.h controls physical embodiment firmware configuration and safety policy.

## Secrets and Credentials

Do not commit Wi-Fi passwords or pairing codes to version control. 

BuddyBot expects a file named `arduino_secrets.h` to exist in the `src/` directory. This file is ignored by Git.

Example `src/arduino_secrets.h`:
```cpp
#pragma once

#define SECRET_WIFI_AP_SSID "MyBuddyBot"
#define SECRET_WIFI_AP_PASSWORD "SuperSecret123"
```

Copy `src/arduino_secrets.example.h` to `src/arduino_secrets.h` and replace both placeholders before building. The build intentionally fails if the file or either required definition is missing; there are no public fallback credentials.

## Hardware Configuration
*   **ACTIVE_BUILD_PROFILE**: Selects the active build profile (e.g. 4-wheel drive, 2-wheel drive, roller drive). See `BUILD_PROFILES.md`.
*   **I2C Settings**: Standard SDA=9, SCL=10, Frequency 400kHz.
*   **Unit 8Servos**: Defaults to I2C address 0x25.
*   **Built-in IMU**: M5StickS3's BMI270 is initialized by M5Unified. BuddyBot samples acceleration (g) and angular velocity (degrees/second) at 10 Hz without using the Grove I2C bus.

## Safety Defaults
The `SAFE DEFAULTS` section of `Config.h` contains hardcoded boundaries that prevent accidental motion. In production, these should generally remain `false`.

*   `ALLOW_MOTOR_ARMING`: Hard switch allowing drive motors to spin.
*   `ENABLE_OBSTACLE_SAFETY`: Enables the Sonic range sensor obstacle stop logic.
*   `REQUIRE_VALID_RANGE_FOR_FORWARD_DRIVE`: Keeps forward motion disabled whenever range data is unavailable, stale, or invalid.
*   `ENABLE_AUTONOMY_AT_BOOT`: Controls if the autonomy loop starts automatically.
*   `ENABLE_CAUTIOUS_ROAM`: Controls if the robot roams automatically.

*There are static assertions to prevent dangerous configurations, such as allowing motor arming while obstacle safety is disabled.*

## Safety Supervisor

BuddyBot starts in `BOOT`, then enters `DISARMED` only after boot diagnostics complete. Motion requires an explicit arm request and may transition to:

- `ARMED`: drive requests are eligible for safety checks.
- `FAULT`: a blocked/invalid forward range reading, drive failure, remote dead-man timeout, or unsafe autonomy IMU state stopped and disarmed the robot.
- `ESTOP`: the local physical button chord was held. Only the local reset chord can leave this state; a separate explicit arm is still required.

Fault and E-stop reasons are persisted in NVS so a reboot cannot silently clear a safety incident. A recovered fault must be explicitly re-armed. Hold both device buttons for `PHYSICAL_ESTOP_HOLD_MS` to stop and latch E-stop; while in E-stop, hold them for `PHYSICAL_ESTOP_RESET_HOLD_MS` to return to `DISARMED`.

`SAFETY_MANUAL_OVERRIDE_MS` gives local/remote manual drive precedence over autonomy. Remote controller disconnect, lease expiry, and drive-watchdog expiry all fault and disarm rather than merely stopping.

## Build Constraints
Due to hardware limitations of the M5StickS3 (ESP32-S3), the code is compiled with `-fno-rtti` to reduce flash and heap usage.
Do not use `String` instances or dynamic heap allocation in fast paths like `RobotAPI` or `WifiControl`.

`.pio/` and `compile_commands.json` are local generated outputs and are intentionally ignored. Regenerate compilation metadata locally with `pio run --target compiledb` when an editor requires it.

## Built-in IMU

The BMI270 supplements, but never replaces, Sonic obstacle safety. Manual drive remains range-gated. Autonomous mode additionally requires a fresh, valid IMU sample within the configured acceleration, tilt, and gyro limits. An unavailable, stale, invalid, or implausible IMU sample faults and disarms autonomy; recovery requires an explicit re-arm.

* **Web UI**: The status panel displays availability and the latest acceleration and rotation vectors. Readings are visible to observers and controllers; no control privilege is required.
* **Serial**: Use `IMU STATUS` or `CMD|IMU|STATUS` to print availability, validity, sample time, acceleration (`imu_accel_g`), and rotation (`imu_gyro_dps`).
* **Status**: `STATUS` includes `imu_available` and `imu_valid`.

The tilt guard assumes the configured Z axis is the upright axis. Mounting orientation and bias must be verified on the assembled robot before enabling autonomy.

## Versioned Interfaces

`CONTROL_PROTOCOL_VERSION`, `CONFIG_SCHEMA_VERSION`, `HARDWARE_MANIFEST_VERSION`, and `SAFETY_POLICY_VERSION` identify the compiled protocol and safety contract. They are included in WebSocket telemetry. This firmware has no operator-writable calibration or credential schema; calibration remains source-controlled until a physically confirmed, atomic provisioning workflow is introduced.
