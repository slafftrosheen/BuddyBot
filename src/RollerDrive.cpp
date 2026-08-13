#include "RollerDrive.h"
#include "Config.h"

RollerDrive::RollerDrive(RollerBus* left, RollerBus* right) : _left(left), _right(right) {}

bool RollerDrive::begin() {
  if (!_left || !_right) return false;
  if (!_left->begin() || !_right->begin()) return false;
  emergencyStop();
  return isConnected();
}

void RollerDrive::update() {
  if (!isConnected()) {
    emergencyStop();
    _armed = false;
    return;
  }

  if (_stopAtMs > 0 && millis() >= _stopAtMs) {
    emergencyStop();
  }
}

bool RollerDrive::isConnected() const {
  return _left && _right && _left->isConnected() && _right->isConnected();
}

bool RollerDrive::isArmed() const {
  return _armed;
}

void RollerDrive::arm() {
  if (!isConnected() || !ALLOW_MOTOR_ARMING) {
    _armed = false;
    return;
  }
  
  if (_left->arm()) {
    if (_right->arm()) {
      _armed = true;
    } else {
      _left->disarm();
      _armed = false;
    }
  } else {
    _armed = false;
  }
}

void RollerDrive::disarm() {
  emergencyStop();
  if (_left) _left->disarm();
  if (_right) _right->disarm();
  _armed = false;
}

void RollerDrive::emergencyStop() {
  if (_left) _left->stop();
  if (_right) _right->stop();
  _mode = DriveMode::STOPPED;
  _stopAtMs = 0;
}

void RollerDrive::drive(const DriveCommand& cmd) {
  if (!_armed || !isConnected()) {
    emergencyStop();
    return;
  }

  int16_t leftMul = 0;
  int16_t rightMul = 0;

  switch (cmd.mode) {
    case DriveMode::STOPPED:
      leftMul = 0; rightMul = 0;
      break;
    case DriveMode::FORWARD:
      leftMul = 1; rightMul = 1;
      break;
    case DriveMode::REVERSE:
      leftMul = -1; rightMul = -1;
      break;
    case DriveMode::TURN_LEFT:
      leftMul = -1; rightMul = 1;
      break;
    case DriveMode::TURN_RIGHT:
      leftMul = 1; rightMul = -1;
      break;
  }

  int16_t lSpeed = leftMul * ROLLER_MAX_RPM;
  int16_t rSpeed = rightMul * ROLLER_MAX_RPM;

  if (ROLLER_LEFT_INVERTED) lSpeed = -lSpeed;
  if (ROLLER_RIGHT_INVERTED) rSpeed = -rSpeed;

  if (!_left->setSpeedRpm(lSpeed) || !_right->setSpeedRpm(rSpeed)) {
    emergencyStop();
    _armed = false;
    return;
  }

  _mode = cmd.mode;
  if (cmd.durationMs > 0) {
    _stopAtMs = millis() + min(cmd.durationMs, SAFE_DRIVE_TIME_MS);
  } else {
    _stopAtMs = millis() + SAFE_DRIVE_TIME_MS;
  }
}

DriveMode RollerDrive::driveMode() const {
  return _mode;
}
