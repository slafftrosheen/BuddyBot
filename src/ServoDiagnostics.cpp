#include "ServoDiagnostics.h"
#include <Arduino.h>

#include "BuildProfiles.h"

namespace {
constexpr bool benchDiagnosticsEnabled() {
#if defined(ENABLE_BENCH_TESTING) && ENABLE_BENCH_TESTING
  return true;
#else
  return false;
#endif
}
}

void ServoDiagnostics::begin(RobotHal* hal) {
  _hal = hal;
  _unlocked = false;
  _testingWheel = false;
  _testRole = ServoRole::UNUSED;
  _lastResult = ServoDiagnosticResult::IDLE;
}

void ServoDiagnostics::update(uint32_t nowMs) {
  if (_unlocked && (nowMs - _unlockedMs > 30000)) {
    stopTest();
    _unlocked = false;
    _lastResult = ServoDiagnosticResult::TIMED_OUT;
  }
}

bool ServoDiagnostics::isUnlocked() const {
  return _unlocked;
}

bool ServoDiagnostics::unlock() {
  if (!benchDiagnosticsEnabled()) {
    _lastResult = ServoDiagnosticResult::BENCH_DISABLED;
    return false;
  }
  if (!_hal) {
    _lastResult = ServoDiagnosticResult::BUS_FAILURE;
    return false;
  }
  if (_hal->drive() && _hal->drive()->isArmed()) {
    _lastResult = ServoDiagnosticResult::DRIVE_ARMED;
    return false;
  }

  _unlocked = true;
  _unlockedMs = millis();
  _lastResult = ServoDiagnosticResult::IDLE;
  return true;
}

void ServoDiagnostics::lock() {
  stopTest();
  _unlocked = false;
  _lastResult = ServoDiagnosticResult::LOCKED;
}

bool ServoDiagnostics::testWheel(ServoRole wheelRole, int16_t pulseUs) {
  if (!_unlocked) {
    _lastResult = ServoDiagnosticResult::LOCKED;
    return false;
  }
  if (!_hal) {
    _lastResult = ServoDiagnosticResult::BUS_FAILURE;
    return false;
  }
  
  if (_testingWheel && _testRole != wheelRole) {
    stopTest();
  }
  
  _unlockedMs = millis();
  
  if (_hal->buildConfig().driveType == DriveControllerType::SERVO8_CONTINUOUS) {
    const ServoChannelConfig* activeConfig = getActiveServoConfig();
    if (activeConfig) {
      for (int i = 0; i < 8; i++) {
        if (activeConfig[i].enabled && activeConfig[i].role == wheelRole && activeConfig[i].continuousRotation) {
          if (pulseUs < activeConfig[i].continuous.minimumUs ||
              pulseUs > activeConfig[i].continuous.maximumUs) {
            _lastResult = ServoDiagnosticResult::INVALID_PULSE;
            return false;
          }
          if (!_hal->writeRawPulse(wheelRole, static_cast<uint16_t>(pulseUs))) {
            _lastResult = ServoDiagnosticResult::BUS_FAILURE;
            return false;
          }
          _testingWheel = true;
          _testRole = wheelRole;
          _lastResult = ServoDiagnosticResult::ACTIVE;
          return true;
        }
      }
    }
  }
  
  _lastResult = ServoDiagnosticResult::INVALID_ROLE;
  return false;
}

void ServoDiagnostics::stopTest() {
  if (_testingWheel && _hal) {
    const ServoChannelConfig* activeConfig = getActiveServoConfig();
    if (activeConfig) {
      for (int i = 0; i < 8; ++i) {
        if (activeConfig[i].enabled &&
            activeConfig[i].role == _testRole &&
            activeConfig[i].continuousRotation) {
          _hal->writeRawPulse(_testRole, activeConfig[i].continuous.stopUs);
          break;
        }
      }
    }
  }
  _testingWheel = false;
  _testRole = ServoRole::UNUSED;
  if (_lastResult == ServoDiagnosticResult::ACTIVE) {
    _lastResult = ServoDiagnosticResult::STOPPED;
  }
}

ServoDiagnosticResult ServoDiagnostics::lastResult() const {
  return _lastResult;
}

const char* ServoDiagnostics::lastResultName() const {
  switch (_lastResult) {
    case ServoDiagnosticResult::IDLE: return "idle";
    case ServoDiagnosticResult::LOCKED: return "locked";
    case ServoDiagnosticResult::BENCH_DISABLED: return "bench_disabled";
    case ServoDiagnosticResult::DRIVE_ARMED: return "drive_armed";
    case ServoDiagnosticResult::INVALID_ROLE: return "invalid_role";
    case ServoDiagnosticResult::INVALID_PULSE: return "invalid_pulse";
    case ServoDiagnosticResult::BUS_FAILURE: return "bus_failure";
    case ServoDiagnosticResult::ACTIVE: return "active";
    case ServoDiagnosticResult::STOPPED: return "stopped";
    case ServoDiagnosticResult::TIMED_OUT: return "timed_out";
  }
  return "unknown";
}
