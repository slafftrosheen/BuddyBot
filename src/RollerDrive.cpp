#include "RollerDrive.h"
#include "Config.h"

RollerDrive::RollerDrive(RollerBus* bus) : _bus(bus) {}

bool RollerDrive::begin() {
  emergencyStop();
  return isConnected();
}

void RollerDrive::update() {
  if (_stopAtMs > 0 && millis() >= _stopAtMs) {
    emergencyStop();
  }
}

bool RollerDrive::isConnected() const {
  return _bus && _bus->isConnected();
}

bool RollerDrive::isArmed() const {
  return _armed;
}

void RollerDrive::arm() {
  if (!isConnected() || !ALLOW_MOTOR_ARMING) {
    _armed = false;
    return;
  }
  _armed = true;
}

void RollerDrive::disarm() {
  emergencyStop();
  _armed = false;
}

void RollerDrive::emergencyStop() {
  if (_bus) {
    _bus->stop();
  }
  _mode = DriveMode::STOPPED;
  _stopAtMs = 0;
}

void RollerDrive::drive(const DriveCommand& cmd) {
  if (!_armed) {
    emergencyStop();
    return;
  }

  // Placeholder: replace with official Roller speed/direction API.
  (void)cmd;

  _mode = cmd.mode;
  _stopAtMs = cmd.durationMs ? millis() + min(cmd.durationMs, SAFE_DRIVE_TIME_MS) : 0;
}

DriveMode RollerDrive::driveMode() const {
  return _mode;
}
