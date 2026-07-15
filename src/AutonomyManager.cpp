#include "AutonomyManager.h"
#include "Config.h"

void AutonomyManager::begin(RobotAPI* robot) {
  _robot = robot;
}

void AutonomyManager::setEnabled(bool enabled) {
  _enabled = enabled;
}

bool AutonomyManager::enabled() const {
  return _enabled;
}

bool AutonomyManager::update(RobotCommand& outCommand) {
  if (!_enabled || !_robot) return false;
  if (millis() < _cooldownUntilMs) return false;

  RangeReading r = _robot->rangeReading();
  if (!r.valid) return false;

  if (r.distanceMm > 0 && r.distanceMm < OBSTACLE_STOP_MM) {
    outCommand = {};
    outCommand.source = ControlSource::AUTONOMY;
    outCommand.kind = CommandKind::STOP;
    _cooldownUntilMs = millis() + 400;
    return true;
  }

  return false;
}
