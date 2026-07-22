# Build Profiles

BuddyBot uses a concept of "Build Profiles" to support different physical hardware configurations while maintaining a single firmware binary. A build profile defines which controllers are used for drive, joints, and accessories.

The active profile is defined in `src/Config.h` via `ACTIVE_BUILD_PROFILE`.

## Profiles

### SERVO8_FOUR_WHEEL_MANIPULATOR (Default)
- **Drive**: Unit 8Servos (4x Continuous Rotation)
- **Head**: Unit 8Servos (Position)
- **Left/Right Arms**: Unit 8Servos (Position)
- **Accessory**: Unit 8Servos (Position)
- **Range Sensor**: Sonic I2C

This is the standard baseline configuration for the `hardware-safety-beta` release.

### SERVO8_TWO_WHEEL_MANIPULATOR
- Same as above, but configured for 2-wheel drive (using legacy mapping).

### STICKY_SERVO_ROVER
- **Drive**: Unit 8Servos (Continuous)
- **Head/Arms/Accessories**: Unit 8Servos (Position)
- **Range Sensor**: Sonic I2C
- Fills out all 8 channels for full articulation testing.

### ROLLER_DRIVE_SERVO_ARMS
- **Drive**: Dual Roller Units
- **Head/Arms**: Unit 8Servos (Position)
- **Range Sensor**: Sonic I2C

### DUAL_ROLLER_DRIVE_ONLY
- **Drive**: Dual Roller Units
- No servos configured.

### ACCESSORY_DEMO_RIG
- **Drive**: None
- **Head/Arms/Accessories**: Unit 8Servos (Position)
- Useful for bench-testing servo channels without activating any continuous drive motors.

### CUSTOM
- Uses the `CUSTOM_BUILD` structure defined in `BuildProfiles.cpp`. 
- Allows developers to define completely bespoke configurations without modifying the standard enumerations.

## Validation

The system runs a `validateBuildConfig()` check to ensure the chosen profile does not map multiple logical servo roles to the same physical hardware channel.

## Usage

You can change the profile by editing `ACTIVE_BUILD_PROFILE` in `src/Config.h`. When running, the `PROFILE LIST` and `PROFILE SHOW` serial commands allow inspecting the current build and available profiles.
