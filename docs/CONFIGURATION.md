# Configuration Guide

BuddyBot uses a unified configuration file (`src/Config.h`) to manage everything from hardware settings to safety defaults.

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

## Build Constraints
Due to hardware limitations of the M5StickS3 (ESP32-S3), the code is compiled with `-fno-rtti` to reduce flash and heap usage.
Do not use `String` instances or dynamic heap allocation in fast paths like `RobotAPI` or `WifiControl`.

## Built-in IMU

The BMI270 is telemetry-only in this release: it does not arm, disarm, stop, or steer the robot. It supplements, but never replaces, the Sonic obstacle safety policy.

* **Web UI**: The status panel displays availability and the latest acceleration and rotation vectors. Readings are visible to observers and controllers; no control privilege is required.
* **Serial**: Use `IMU STATUS` or `CMD|IMU|STATUS` to print availability, validity, sample time, acceleration (`imu_accel_g`), and rotation (`imu_gyro_dps`).
* **Status**: `STATUS` includes `imu_available` and `imu_valid`.

The reported axes follow the orientation configured by M5Unified. Mounting orientation and bias must be verified on the assembled robot before using these readings for future tilt, collision, or motion-control features.
