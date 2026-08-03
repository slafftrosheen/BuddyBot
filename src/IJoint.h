#pragma once
#include <Arduino.h>
#include "JointMotion.h"

class IJoint {
public:
  virtual ~IJoint() = default;
  virtual bool begin() = 0;
  virtual bool isConnected() const = 0;
  virtual void moveTo(int16_t value, uint16_t durationMs = 0) = 0;
  virtual int16_t current() const = 0;
  virtual void rest() = 0;
  virtual int16_t target() const { return current(); }
  virtual bool motionActive() const { return false; }
  virtual void cancelMotion() {}
  virtual JointMotionState motionState() const {
    return isConnected() ? JointMotionState::IDLE : JointMotionState::UNAVAILABLE;
  }
};
