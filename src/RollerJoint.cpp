#include "RollerJoint.h"
#include "HalTypes.h"

RollerJoint::RollerJoint(RollerBus* bus, const JointLimits& limits)
: _bus(bus), _limits(limits), _current(limits.restValue) {}

bool RollerJoint::begin() {
  rest();
  return isConnected();
}

bool RollerJoint::isConnected() const {
  return _bus && _bus->isConnected();
}

void RollerJoint::moveTo(int16_t value, uint16_t durationMs) {
  if (!_bus || !_bus->isConnected()) return;
  _current = constrain(value, _limits.minValue, _limits.maxValue);
  if (_bus) {
    _bus->setPositionCounts(_current);
  }
}

int16_t RollerJoint::current() const {
  return _current;
}

void RollerJoint::rest() {
  moveTo(_limits.restValue);
}
