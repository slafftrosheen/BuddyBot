#pragma once

#include <stdint.h>

enum class SafetyState : uint8_t {
  BOOT,
  DISARMED,
  ARMED,
  FAULT,
  ESTOP
};

enum class SafetyFault : uint8_t {
  NONE,
  BOOT_INCOMPLETE,
  HARDWARE_UNAVAILABLE,
  DRIVE_UNAVAILABLE,
  OBSTACLE_BLOCKED,
  RANGE_SENSOR_INVALID,
  IMU_UNAVAILABLE,
  IMU_INVALID,
  IMU_STALE,
  IMU_TILT,
  IMU_ACCELERATION,
  IMU_GYRO,
  CONTROLLER_DISCONNECTED,
  CONTROLLER_LEASE_EXPIRED,
  DRIVE_WATCHDOG,
  AUTONOMY_TIMEOUT,
  PHYSICAL_ESTOP
};

struct SafetyInputs {
  bool bootComplete = false;
  bool hardwareReady = false;
  bool driveAvailable = false;
  bool rangeValid = false;
  bool forwardMotionBlocked = true;
  bool driveActive = false;
  bool driveForward = false;
  bool imuAvailable = false;
  bool imuValid = false;
  uint32_t imuSampleTimeMs = 0;
  float accelXG = 0.0f;
  float accelYG = 0.0f;
  float accelZG = 0.0f;
  float gyroXDps = 0.0f;
  float gyroYDps = 0.0f;
  float gyroZDps = 0.0f;
};

struct SafetyPolicyConfig {
  uint16_t manualOverrideMs = 600;
  uint16_t imuMaxSampleAgeMs = 500;
  float imuMinimumAccelG = 0.25f;
  float imuMaximumAccelG = 3.0f;
  float imuMaximumTiltDeg = 35.0f;
  float imuMaximumGyroDps = 720.0f;
};

class SafetySupervisor {
 public:
  void configure(const SafetyPolicyConfig& config);
  void begin();

  void completeBoot(const SafetyInputs& inputs, uint32_t nowMs);
  void update(const SafetyInputs& inputs, uint32_t nowMs);

  bool requestArm(const SafetyInputs& inputs, uint32_t nowMs);
  void requestDisarm(uint32_t nowMs);
  bool requestDrive(bool forward, bool autonomous, uint32_t nowMs);
  bool requestAutonomy(bool enabled, uint32_t nowMs);

  void emergencyStop(SafetyFault fault, uint32_t nowMs);
  void physicalEstop(uint32_t nowMs);
  bool clearPhysicalEstop(const SafetyInputs& inputs, uint32_t nowMs);
  void notifyArmFailed(uint32_t nowMs);

  bool mayEnableDrive() const;
  bool mayMoveManipulators() const;
  bool autonomyEnabled() const;
  bool autonomyMotionAllowed(uint32_t nowMs) const;
  bool consumeDisarmRequest();

  SafetyState state() const;
  SafetyFault fault() const;
  uint32_t stateChangedAtMs() const;
  uint32_t manualOverrideUntilMs() const;
  const SafetyInputs& inputs() const;

  static const char* stateName(SafetyState state);
  static const char* faultName(SafetyFault fault);

 private:
  SafetyPolicyConfig _config;
  SafetyInputs _inputs;
  SafetyState _state = SafetyState::BOOT;
  SafetyFault _fault = SafetyFault::NONE;
  SafetyState _restoredState = SafetyState::BOOT;
  SafetyFault _restoredFault = SafetyFault::NONE;
  uint32_t _stateChangedAtMs = 0;
  uint32_t _manualOverrideUntilMs = 0;
  bool _autonomyEnabled = false;
  bool _disarmRequested = false;

  SafetyFault faultForArm() const;
  SafetyFault faultForDrive(bool forward, bool autonomous, uint32_t nowMs) const;
  SafetyFault faultForAutonomy(uint32_t nowMs) const;
  void latchFault(SafetyFault fault, uint32_t nowMs);
  void persistLatchedState() const;
  void clearPersistedState() const;
  void loadPersistedState();
};
