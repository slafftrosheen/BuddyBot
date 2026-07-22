#include "ServoJoint.h"

ServoJoint::ServoJoint(Servo8Bus* bus, ServoRole role)
    : _bus(bus), _role(role) {}

bool ServoJoint::begin() {
  if (!isConnected()) return false;
  const ServoChannelConfig* cfg = _bus->configForRole(_role);
  if (cfg && cfg->enabled && !cfg->continuousRotation) {
    _motion.begin(cfg->positional.restDeg);
    moveTo(cfg->positional.restDeg);
  }
  return true;
}

bool ServoJoint::isConnected() const {
  return _bus && _bus->isConnected() && _bus->hasRole(_role);
}

void ServoJoint::moveTo(int16_t value, uint16_t durationMs) {
  if (durationMs > 0) {
    moveTo(value, durationMs, JointEasing::EASE_IN_OUT);
  } else {
    cancelMotion();
    _internalMove(value);
  }
}

void ServoJoint::_internalMove(int16_t value) {
  if (!isConnected()) return;

  const ServoChannelConfig* cfg = _bus->configForRole(_role);
  if (!cfg || !cfg->enabled || cfg->continuousRotation) return;

  uint8_t constrained = constrain(value, cfg->positional.minimumDeg, cfg->positional.maximumDeg);
  
  // Logical state update
  if (!_motion.active()) {
    _motion.begin(constrained);
  }
  
  if (cfg->positional.inverted) {
    constrained = 180 - constrained;
  }

  _bus->writeAngle(cfg->channel, constrained);
}

bool ServoJoint::moveTo(int16_t targetDeg, uint16_t durationMs, JointEasing easing) {
  if (!isConnected()) return false;
  const ServoChannelConfig* cfg = _bus->configForRole(_role);
  if (!cfg || !cfg->enabled || cfg->continuousRotation) return false;
  
  int16_t constrained = constrain(targetDeg, cfg->positional.minimumDeg, cfg->positional.maximumDeg);
  
  if (durationMs > 0) {
    if (durationMs < SERVO_MANIPULATOR_STEP_MIN_MS) durationMs = SERVO_MANIPULATOR_STEP_MIN_MS;
    if (durationMs > SERVO_MANIPULATOR_STEP_MAX_MS) durationMs = SERVO_MANIPULATOR_STEP_MAX_MS;
  }

  return _motion.start(_motion.currentValue(), constrained, durationMs, easing);
}

void ServoJoint::update(uint32_t nowMs) {
  if (!_motion.active()) return;
  
  if (nowMs - _lastUpdateMs < SERVO_JOINT_UPDATE_INTERVAL_MS) {
    return;
  }
  _lastUpdateMs = nowMs;
  
  _motion.update(nowMs);
  moveTo(_motion.currentValue());
}

void ServoJoint::cancelMotion() {
  _motion.cancel();
}

bool ServoJoint::motionActive() const {
  return _motion.active();
}

JointMotionState ServoJoint::motionState() const {
  return _motion.state();
}

int16_t ServoJoint::target() const {
  return _motion.currentTarget();
}

int16_t ServoJoint::current() const {
  return _motion.currentValue();
}

void ServoJoint::rest() {
  if (!isConnected()) return;
  const ServoChannelConfig* cfg = _bus->configForRole(_role);
  if (cfg && cfg->enabled && !cfg->continuousRotation) {
    moveTo(cfg->positional.restDeg, 600, JointEasing::EASE_IN_OUT);
  }
}
