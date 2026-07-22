#include "JointMotion.h"
#include <Arduino.h>

void JointMotion::begin(int16_t initialDeg) {
  _current = initialDeg;
  _from = initialDeg;
  _to = initialDeg;
  _state = JointMotionState::IDLE;
}

bool JointMotion::start(int16_t fromDeg, int16_t toDeg, uint16_t durationMs, JointEasing easing) {
  _from = fromDeg;
  _to = toDeg;
  _current = fromDeg;
  _easing = easing;
  
  if (durationMs == 0) {
    _current = toDeg;
    _state = JointMotionState::COMPLETE;
    return true;
  }
  
  _durationMs = durationMs;
  _startedMs = millis();
  _state = JointMotionState::MOVING;
  
  return true;
}

void JointMotion::update(uint32_t nowMs) {
  if (_state != JointMotionState::MOVING) {
    return;
  }
  
  uint32_t elapsed = nowMs - _startedMs;
  if (elapsed >= _durationMs) {
    _current = _to;
    _state = JointMotionState::COMPLETE;
    return;
  }
  
  // Integer division for progress (0..1024)
  uint32_t progress1024 = (elapsed * 1024) / _durationMs;
  if (progress1024 > 1024) progress1024 = 1024;
  
  uint32_t factor1024 = progress1024;
  
  switch (_easing) {
    case JointEasing::LINEAR:
      break;
    case JointEasing::EASE_IN_OUT: {
      // Very simple integer approximation of ease-in-out curve
      // For t in 0..1024
      // if t < 512, return 2*t^2 
      // else return 1 - 2*(1-t)^2
      if (progress1024 < 512) {
        factor1024 = (2 * progress1024 * progress1024) / 1024;
      } else {
        uint32_t inv = 1024 - progress1024;
        factor1024 = 1024 - ((2 * inv * inv) / 1024);
      }
      break;
    }
    case JointEasing::EASE_OUT: {
      // t * (2 - t)
      factor1024 = (progress1024 * (2048 - progress1024)) / 1024;
      break;
    }
  }
  
  int32_t diff = _to - _from;
  _current = _from + (diff * (int32_t)factor1024) / 1024;
}

void JointMotion::cancel() {
  if (_state == JointMotionState::MOVING) {
    _state = JointMotionState::CANCELLED;
  }
}

bool JointMotion::active() const {
  return _state == JointMotionState::MOVING;
}

bool JointMotion::complete() const {
  return _state == JointMotionState::COMPLETE;
}

JointMotionState JointMotion::state() const {
  return _state;
}

int16_t JointMotion::currentTarget() const {
  return _to;
}

int16_t JointMotion::currentValue() const {
  return _current;
}
