#include "ServoDrive.h"
#include "Config.h"

ServoDrive::ServoDrive(Servo8Bus* bus) : _bus(bus) {}

bool ServoDrive::begin() {
  emergencyStop();
  return isConnected();
}

void ServoDrive::update() {
  if (_stopAtMs > 0 && millis() >= _stopAtMs) {
    emergencyStop();
  }
}

bool ServoDrive::isConnected() const {
  return _bus && _bus->isConnected();
}

bool ServoDrive::isArmed() const {
  return _armed;
}

void ServoDrive::arm() {
  if (!isConnected() || !ALLOW_MOTOR_ARMING) {
    _armed = false;
    return;
  }
  wheels(LEFT_STOP_US, RIGHT_STOP_US);
  _armed = true;
}

void ServoDrive::disarm() {
  emergencyStop();
  _armed = false;
}

void ServoDrive::emergencyStop() {
  if (isConnected()) {
    wheels(LEFT_STOP_US, RIGHT_STOP_US);
  }
  _mode = DriveMode::STOPPED;
  _stopAtMs = 0;
}

void ServoDrive::wheels(uint16_t leftUs, uint16_t rightUs) {
  _bus->setPulse(LEFT_WHEEL_CHANNEL, leftUs);
  _bus->setPulse(RIGHT_WHEEL_CHANNEL, rightUs);
}

void ServoDrive::drive(const DriveCommand& cmd) {
  if (!_armed) {
    emergencyStop();
    return;
  }

  switch (cmd.mode) {
    case DriveMode::FORWARD:    wheels(LEFT_FORWARD_US, RIGHT_FORWARD_US); break;
    case DriveMode::REVERSE:    wheels(LEFT_REVERSE_US, RIGHT_REVERSE_US); break;
    case DriveMode::TURN_LEFT:  wheels(LEFT_REVERSE_US, RIGHT_FORWARD_US); break;
    case DriveMode::TURN_RIGHT: wheels(LEFT_FORWARD_US, RIGHT_REVERSE_US); break;
    default: emergencyStop(); return;
  }

  _mode = cmd.mode;
  _stopAtMs = cmd.durationMs ? millis() + min(cmd.durationMs, SAFE_DRIVE_TIME_MS) : 0;
}

DriveMode ServoDrive::driveMode() const {
  return _mode;
}
