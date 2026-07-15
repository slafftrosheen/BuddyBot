#include "ServoJoint.h"
#include "HalTypes.h"

ServoJoint::ServoJoint(Servo8Bus* bus, uint8_t channel, const JointLimits& limits)
: _bus(bus), _channel(channel), _limits(limits), _current(limits.restValue) {}

bool ServoJoint::begin() {
  rest();
  return isConnected();
}

bool ServoJoint::isConnected() const {
  return _bus && _bus->isConnected();
}

void ServoJoint::moveTo(int16_t value) {
  if (!isConnected()) return;
  _current = constrain(value, _limits.minValue, _limits.maxValue);
  _bus->setAngle(_channel, (uint8_t)_current);
}

int16_t ServoJoint::current() const {
  return _current;
}

void ServoJoint::rest() {
  moveTo(_limits.restValue);
}
