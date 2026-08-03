#pragma once
#include "IJoint.h"
#include "HalTypes.h"
#include "Servo8Bus.h"
#include "JointMotion.h"

class ServoJoint : public IJoint {
public:
  ServoJoint(Servo8Bus* bus, ServoRole role);

  bool begin() override;
  bool isConnected() const override;
  
  // Retained for backward compat (immediate jump if durationMs == 0)
  void moveTo(int16_t value, uint16_t durationMs = 0) override;
  int16_t current() const override;
  void rest() override;

  // New JointMotion API with explicit easing
  bool moveTo(int16_t targetDeg, uint16_t durationMs, JointEasing easing);
  void update(uint32_t nowMs);
  void cancelMotion() override;
  bool motionActive() const override;
  JointMotionState motionState() const override;
  int16_t target() const override;

private:
  Servo8Bus* _bus = nullptr;
  ServoRole _role;
  
  void _internalMove(int16_t value);
  
  JointMotion _motion;
  uint32_t _lastUpdateMs = 0;
};
