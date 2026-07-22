#include "ObstacleSafety.h"

void ObstacleSafety::begin() {
  _state = ObstacleSafetyState::SENSOR_UNAVAILABLE;
  _lastStopReason = SafetyStopReason::NONE;
  _lastStopMs = 0;
  _rangeMm = 0;
  _rangeValid = false;
  _clearSinceMs = 0;
}

void ObstacleSafety::update(const RangeReading& range, uint32_t nowMs) {
  _rangeValid = range.valid;
  _rangeMm = range.distanceMm;
  
  if (!_rangeValid) {
    _state = ObstacleSafetyState::SENSOR_UNAVAILABLE;
    _clearSinceMs = 0;
    return;
  }
  
  if (_rangeMm < OBSTACLE_EMERGENCY_STOP_MM) {
    _state = ObstacleSafetyState::BLOCKED;
    _clearSinceMs = 0;
  } else if (_rangeMm < OBSTACLE_CLEAR_MM) {
    if (_state == ObstacleSafetyState::BLOCKED) {
      // Must reach CLEAR_MM to leave BLOCKED state
      _clearSinceMs = 0;
    } else {
      _state = ObstacleSafetyState::CAUTION;
      _clearSinceMs = 0;
    }
  } else {
    if (_state != ObstacleSafetyState::CLEAR) {
      if (_clearSinceMs == 0) {
        _clearSinceMs = nowMs;
      }
      if (nowMs - _clearSinceMs >= OBSTACLE_HYSTERESIS_MS) {
        _state = ObstacleSafetyState::CLEAR;
      }
    } else {
      _clearSinceMs = nowMs;
    }
  }
}

bool ObstacleSafety::blocksForwardMotion() const {
  if (!ENABLE_OBSTACLE_SAFETY) return false;
  if (_state == ObstacleSafetyState::BLOCKED) return true;
  if (REQUIRE_VALID_RANGE_FOR_FORWARD_DRIVE && _state == ObstacleSafetyState::SENSOR_UNAVAILABLE) return true;
  return false;
}

bool ObstacleSafety::allowsDrive(DriveMode requested) const {
  if (!ENABLE_OBSTACLE_SAFETY) return true;
  if (requested == DriveMode::FORWARD && blocksForwardMotion()) return false;
  return true;
}

ObstacleSafetyStatus ObstacleSafety::status() const {
  ObstacleSafetyStatus st;
  st.state = _state;
  st.lastStopReason = _lastStopReason;
  st.lastStopMs = _lastStopMs;
  st.filteredRangeMm = _rangeMm;
  st.rangeValid = _rangeValid;
  st.forwardMotionBlocked = blocksForwardMotion();
  return st;
}

void ObstacleSafety::recordExternalStop(SafetyStopReason reason, uint32_t nowMs) {
  _lastStopReason = reason;
  _lastStopMs = nowMs;
}

void ObstacleSafety::clearExternalStopIfSafe() {
  if (!blocksForwardMotion()) {
    _lastStopReason = SafetyStopReason::NONE;
  }
}
