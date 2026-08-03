#include "SafetySupervisor.h"

#include <math.h>

#if defined(ARDUINO)
#include <Preferences.h>
#endif

namespace {
constexpr char SAFETY_PREFERENCES_NAMESPACE[] = "buddybot";
constexpr char SAFETY_STATE_KEY[] = "safety_state";
constexpr char SAFETY_FAULT_KEY[] = "safety_fault";

bool isFinite(float value) {
  return isfinite(value);
}
}

void SafetySupervisor::configure(const SafetyPolicyConfig& config) {
  _config = config;
}

void SafetySupervisor::begin() {
  _state = SafetyState::BOOT;
  _fault = SafetyFault::NONE;
  _stateChangedAtMs = 0;
  _manualOverrideUntilMs = 0;
  _autonomyEnabled = false;
  _disarmRequested = false;
  _restoredState = SafetyState::BOOT;
  _restoredFault = SafetyFault::NONE;
  loadPersistedState();
}

void SafetySupervisor::completeBoot(const SafetyInputs& inputs, uint32_t nowMs) {
  _inputs = inputs;
  _inputs.bootComplete = true;

  if (_restoredState == SafetyState::ESTOP) {
    _state = SafetyState::ESTOP;
    _fault = SafetyFault::PHYSICAL_ESTOP;
    _disarmRequested = true;
    _stateChangedAtMs = nowMs;
    return;
  }

  if (_restoredState == SafetyState::FAULT) {
    _state = SafetyState::FAULT;
    _fault = _restoredFault == SafetyFault::NONE ? SafetyFault::HARDWARE_UNAVAILABLE : _restoredFault;
    _disarmRequested = true;
    _stateChangedAtMs = nowMs;
    return;
  }

  const SafetyFault fault = faultForArm();
  if (fault != SafetyFault::NONE) {
    latchFault(fault, nowMs);
    return;
  }

  _state = SafetyState::DISARMED;
  _fault = SafetyFault::NONE;
  _stateChangedAtMs = nowMs;
}

void SafetySupervisor::update(const SafetyInputs& inputs, uint32_t nowMs) {
  _inputs = inputs;

  if (_state == SafetyState::BOOT || _state == SafetyState::ESTOP || _state == SafetyState::FAULT) {
    return;
  }

  if (!_inputs.hardwareReady || !_inputs.driveAvailable) {
    latchFault(SafetyFault::DRIVE_UNAVAILABLE, nowMs);
    return;
  }

  if (_state == SafetyState::ARMED && _inputs.driveForward) {
    const SafetyFault fault = faultForDrive(true, false, nowMs);
    if (fault != SafetyFault::NONE) {
      latchFault(fault, nowMs);
      return;
    }
  }

  if (_state == SafetyState::ARMED && _autonomyEnabled) {
    const SafetyFault fault = faultForAutonomy(nowMs);
    if (fault != SafetyFault::NONE) {
      latchFault(fault, nowMs);
    }
  }
}

bool SafetySupervisor::requestArm(const SafetyInputs& inputs, uint32_t nowMs) {
  _inputs = inputs;

  if (_state == SafetyState::BOOT || _state == SafetyState::ESTOP) {
    return false;
  }

  const SafetyFault fault = faultForArm();
  if (fault != SafetyFault::NONE) {
    latchFault(fault, nowMs);
    return false;
  }

  _state = SafetyState::ARMED;
  _fault = SafetyFault::NONE;
  _stateChangedAtMs = nowMs;
  _autonomyEnabled = false;
  _disarmRequested = false;
  clearPersistedState();
  return true;
}

void SafetySupervisor::requestDisarm(uint32_t nowMs) {
  if (_state == SafetyState::ESTOP || _state == SafetyState::FAULT) {
    _disarmRequested = true;
    return;
  }

  _state = SafetyState::DISARMED;
  _fault = SafetyFault::NONE;
  _stateChangedAtMs = nowMs;
  _manualOverrideUntilMs = 0;
  _autonomyEnabled = false;
  _disarmRequested = false;
}

bool SafetySupervisor::requestDrive(bool forward, bool autonomous, uint32_t nowMs) {
  if (_state != SafetyState::ARMED) {
    return false;
  }

  const SafetyFault fault = faultForDrive(forward, autonomous, nowMs);
  if (fault != SafetyFault::NONE) {
    latchFault(fault, nowMs);
    return false;
  }

  if (autonomous) {
    if (!_autonomyEnabled || !autonomyMotionAllowed(nowMs)) {
      return false;
    }
  } else {
    _manualOverrideUntilMs = nowMs + _config.manualOverrideMs;
  }

  return true;
}

bool SafetySupervisor::requestAutonomy(bool enabled, uint32_t nowMs) {
  if (!enabled) {
    _autonomyEnabled = false;
    return true;
  }

  if (_state != SafetyState::ARMED) {
    return false;
  }

  const SafetyFault fault = faultForAutonomy(nowMs);
  if (fault != SafetyFault::NONE) {
    latchFault(fault, nowMs);
    return false;
  }

  _autonomyEnabled = true;
  return true;
}

void SafetySupervisor::emergencyStop(SafetyFault fault, uint32_t nowMs) {
  if (_state == SafetyState::ESTOP) {
    _disarmRequested = true;
    return;
  }

  latchFault(fault == SafetyFault::NONE ? SafetyFault::DRIVE_WATCHDOG : fault, nowMs);
}

void SafetySupervisor::physicalEstop(uint32_t nowMs) {
  _state = SafetyState::ESTOP;
  _fault = SafetyFault::PHYSICAL_ESTOP;
  _stateChangedAtMs = nowMs;
  _manualOverrideUntilMs = 0;
  _autonomyEnabled = false;
  _disarmRequested = true;
  persistLatchedState();
}

bool SafetySupervisor::clearPhysicalEstop(const SafetyInputs& inputs, uint32_t nowMs) {
  _inputs = inputs;
  if (_state != SafetyState::ESTOP || !_inputs.bootComplete || !_inputs.hardwareReady || !_inputs.driveAvailable) {
    return false;
  }

  _state = SafetyState::DISARMED;
  _fault = SafetyFault::NONE;
  _stateChangedAtMs = nowMs;
  _disarmRequested = false;
  clearPersistedState();
  return true;
}

void SafetySupervisor::notifyArmFailed(uint32_t nowMs) {
  latchFault(SafetyFault::DRIVE_UNAVAILABLE, nowMs);
}

bool SafetySupervisor::mayEnableDrive() const {
  return _state == SafetyState::ARMED;
}

bool SafetySupervisor::mayMoveManipulators() const {
  return _state == SafetyState::DISARMED || _state == SafetyState::ARMED;
}

bool SafetySupervisor::autonomyEnabled() const {
  return _autonomyEnabled;
}

bool SafetySupervisor::autonomyMotionAllowed(uint32_t nowMs) const {
  return _state == SafetyState::ARMED &&
         _autonomyEnabled &&
         static_cast<int32_t>(nowMs - _manualOverrideUntilMs) >= 0;
}

bool SafetySupervisor::consumeDisarmRequest() {
  const bool requested = _disarmRequested;
  _disarmRequested = false;
  return requested;
}

SafetyState SafetySupervisor::state() const {
  return _state;
}

SafetyFault SafetySupervisor::fault() const {
  return _fault;
}

uint32_t SafetySupervisor::stateChangedAtMs() const {
  return _stateChangedAtMs;
}

uint32_t SafetySupervisor::manualOverrideUntilMs() const {
  return _manualOverrideUntilMs;
}

const SafetyInputs& SafetySupervisor::inputs() const {
  return _inputs;
}

const char* SafetySupervisor::stateName(SafetyState state) {
  switch (state) {
    case SafetyState::BOOT: return "boot";
    case SafetyState::DISARMED: return "disarmed";
    case SafetyState::ARMED: return "armed";
    case SafetyState::FAULT: return "fault";
    case SafetyState::ESTOP: return "estop";
  }
  return "unknown";
}

const char* SafetySupervisor::faultName(SafetyFault fault) {
  switch (fault) {
    case SafetyFault::NONE: return "none";
    case SafetyFault::BOOT_INCOMPLETE: return "boot_incomplete";
    case SafetyFault::HARDWARE_UNAVAILABLE: return "hardware_unavailable";
    case SafetyFault::DRIVE_UNAVAILABLE: return "drive_unavailable";
    case SafetyFault::OBSTACLE_BLOCKED: return "obstacle_blocked";
    case SafetyFault::RANGE_SENSOR_INVALID: return "range_sensor_invalid";
    case SafetyFault::IMU_UNAVAILABLE: return "imu_unavailable";
    case SafetyFault::IMU_INVALID: return "imu_invalid";
    case SafetyFault::IMU_STALE: return "imu_stale";
    case SafetyFault::IMU_TILT: return "imu_tilt";
    case SafetyFault::IMU_ACCELERATION: return "imu_acceleration";
    case SafetyFault::IMU_GYRO: return "imu_gyro";
    case SafetyFault::CONTROLLER_DISCONNECTED: return "controller_disconnected";
    case SafetyFault::CONTROLLER_LEASE_EXPIRED: return "controller_lease_expired";
    case SafetyFault::DRIVE_WATCHDOG: return "drive_watchdog";
    case SafetyFault::AUTONOMY_TIMEOUT: return "autonomy_timeout";
    case SafetyFault::PHYSICAL_ESTOP: return "physical_estop";
  }
  return "unknown";
}

SafetyFault SafetySupervisor::faultForArm() const {
  if (!_inputs.bootComplete) {
    return SafetyFault::BOOT_INCOMPLETE;
  }
  if (!_inputs.hardwareReady) {
    return SafetyFault::HARDWARE_UNAVAILABLE;
  }
  if (!_inputs.driveAvailable) {
    return SafetyFault::DRIVE_UNAVAILABLE;
  }
  return SafetyFault::NONE;
}

SafetyFault SafetySupervisor::faultForDrive(bool forward, bool autonomous, uint32_t nowMs) const {
  if (!_inputs.hardwareReady || !_inputs.driveAvailable) {
    return SafetyFault::DRIVE_UNAVAILABLE;
  }

  if (forward) {
    if (!_inputs.rangeValid) {
      return SafetyFault::RANGE_SENSOR_INVALID;
    }
    if (_inputs.forwardMotionBlocked) {
      return SafetyFault::OBSTACLE_BLOCKED;
    }
  }

  if (autonomous) {
    return faultForAutonomy(nowMs);
  }

  return SafetyFault::NONE;
}

SafetyFault SafetySupervisor::faultForAutonomy(uint32_t nowMs) const {
  if (!_inputs.rangeValid) {
    return SafetyFault::RANGE_SENSOR_INVALID;
  }
  if (_inputs.forwardMotionBlocked) {
    return SafetyFault::OBSTACLE_BLOCKED;
  }
  if (!_inputs.imuAvailable) {
    return SafetyFault::IMU_UNAVAILABLE;
  }
  if (!_inputs.imuValid) {
    return SafetyFault::IMU_INVALID;
  }
  if (_inputs.imuSampleTimeMs == 0 || nowMs - _inputs.imuSampleTimeMs > _config.imuMaxSampleAgeMs) {
    return SafetyFault::IMU_STALE;
  }

  if (!isFinite(_inputs.accelXG) || !isFinite(_inputs.accelYG) || !isFinite(_inputs.accelZG) ||
      !isFinite(_inputs.gyroXDps) || !isFinite(_inputs.gyroYDps) || !isFinite(_inputs.gyroZDps)) {
    return SafetyFault::IMU_INVALID;
  }

  const float accelMagnitude = sqrtf(
    _inputs.accelXG * _inputs.accelXG +
    _inputs.accelYG * _inputs.accelYG +
    _inputs.accelZG * _inputs.accelZG
  );
  if (accelMagnitude < _config.imuMinimumAccelG || accelMagnitude > _config.imuMaximumAccelG) {
    return SafetyFault::IMU_ACCELERATION;
  }

  float verticalFraction = fabsf(_inputs.accelZG) / accelMagnitude;
  if (verticalFraction > 1.0f) {
    verticalFraction = 1.0f;
  }
  const float tiltDeg = acosf(verticalFraction) * 57.2957795f;
  if (tiltDeg > _config.imuMaximumTiltDeg) {
    return SafetyFault::IMU_TILT;
  }

  if (fabsf(_inputs.gyroXDps) > _config.imuMaximumGyroDps ||
      fabsf(_inputs.gyroYDps) > _config.imuMaximumGyroDps ||
      fabsf(_inputs.gyroZDps) > _config.imuMaximumGyroDps) {
    return SafetyFault::IMU_GYRO;
  }

  return SafetyFault::NONE;
}

void SafetySupervisor::latchFault(SafetyFault fault, uint32_t nowMs) {
  if (_state == SafetyState::ESTOP) {
    _disarmRequested = true;
    return;
  }

  if (_state == SafetyState::FAULT && _fault == fault) {
    _disarmRequested = true;
    return;
  }

  _state = SafetyState::FAULT;
  _fault = fault;
  _stateChangedAtMs = nowMs;
  _manualOverrideUntilMs = 0;
  _autonomyEnabled = false;
  _disarmRequested = true;
  persistLatchedState();
}

void SafetySupervisor::persistLatchedState() const {
#if defined(ARDUINO)
  Preferences preferences;
  if (preferences.begin(SAFETY_PREFERENCES_NAMESPACE, false)) {
    preferences.putUChar(SAFETY_STATE_KEY, static_cast<uint8_t>(_state));
    preferences.putUChar(SAFETY_FAULT_KEY, static_cast<uint8_t>(_fault));
    preferences.end();
  }
#endif
}

void SafetySupervisor::clearPersistedState() const {
#if defined(ARDUINO)
  Preferences preferences;
  if (preferences.begin(SAFETY_PREFERENCES_NAMESPACE, false)) {
    preferences.remove(SAFETY_STATE_KEY);
    preferences.remove(SAFETY_FAULT_KEY);
    preferences.end();
  }
#endif
}

void SafetySupervisor::loadPersistedState() {
#if defined(ARDUINO)
  Preferences preferences;
  if (!preferences.begin(SAFETY_PREFERENCES_NAMESPACE, true)) {
    return;
  }

  const uint8_t state = preferences.getUChar(SAFETY_STATE_KEY, static_cast<uint8_t>(SafetyState::BOOT));
  const uint8_t fault = preferences.getUChar(SAFETY_FAULT_KEY, static_cast<uint8_t>(SafetyFault::NONE));
  preferences.end();

  if (state == static_cast<uint8_t>(SafetyState::FAULT)) {
    _restoredState = SafetyState::FAULT;
    _restoredFault = fault <= static_cast<uint8_t>(SafetyFault::PHYSICAL_ESTOP)
      ? static_cast<SafetyFault>(fault)
      : SafetyFault::HARDWARE_UNAVAILABLE;
  } else if (state == static_cast<uint8_t>(SafetyState::ESTOP)) {
    _restoredState = SafetyState::ESTOP;
    _restoredFault = SafetyFault::PHYSICAL_ESTOP;
  }
#endif
}
