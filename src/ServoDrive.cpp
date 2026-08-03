#include "ServoDrive.h"
#include "Config.h"

ServoDrive::ServoDrive(Servo8Bus* bus, const ServoDriveLayout& layout) : _bus(bus), _layout(layout) {}

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

bool ServoDrive::isFourWheel() const {
  return _layout.isFourWheel;
}

void ServoDrive::arm() {
  if (!isConnected() || !ALLOW_MOTOR_ARMING) {
    _armed = false;
    return;
  }
  stopDrive();
  _armed = true;
}

void ServoDrive::disarm() {
  emergencyStop();
  _armed = false;
}

void ServoDrive::emergencyStop() {
  if (isConnected()) {
    stopDrive();
  }
  _mode = DriveMode::STOPPED;
  _stopAtMs = 0;
}

void ServoDrive::applyDrivePulse(ServoRole role, bool forward) {
  if (role == ServoRole::UNUSED) return;
  const ServoChannelConfig* cfg = _bus->configForRole(role);
  if (!cfg || !cfg->enabled || !cfg->continuousRotation) return;

  uint16_t pulse = forward ? cfg->continuous.forwardUs : cfg->continuous.reverseUs;
  _bus->writePulse(cfg->channel, pulse);
}

void ServoDrive::setLeftDrive(bool forward) {
  applyDrivePulse(_layout.frontLeft, forward);
  if (_layout.isFourWheel) {
    applyDrivePulse(_layout.rearLeft, forward);
  }
}

void ServoDrive::setRightDrive(bool forward) {
  applyDrivePulse(_layout.frontRight, forward);
  if (_layout.isFourWheel) {
    applyDrivePulse(_layout.rearRight, forward);
  }
}

void ServoDrive::stopDrive() {
  auto stopRole = [this](ServoRole role) {
    if (role == ServoRole::UNUSED) return;
    const ServoChannelConfig* cfg = _bus->configForRole(role);
    if (cfg && cfg->enabled && cfg->continuousRotation) {
      _bus->writePulse(cfg->channel, cfg->continuous.stopUs);
    }
  };

  stopRole(_layout.frontLeft);
  stopRole(_layout.frontRight);
  if (_layout.isFourWheel) {
    stopRole(_layout.rearLeft);
    stopRole(_layout.rearRight);
  }
}

void ServoDrive::drive(const DriveCommand& cmd) {
  if (!_armed) {
    emergencyStop();
    return;
  }

  switch (cmd.mode) {
    case DriveMode::FORWARD:
      setLeftDrive(true);
      setRightDrive(true);
      break;
    case DriveMode::REVERSE:
      setLeftDrive(false);
      setRightDrive(false);
      break;
    case DriveMode::TURN_LEFT:
      setLeftDrive(false);
      setRightDrive(true);
      break;
    case DriveMode::TURN_RIGHT:
      setLeftDrive(true);
      setRightDrive(false);
      break;
    default:
      emergencyStop();
      return;
  }

  _mode = cmd.mode;
  const uint16_t boundedDuration = cmd.durationMs > 0
    ? min(cmd.durationMs, SAFE_DRIVE_TIME_MS)
    : SAFE_DRIVE_TIME_MS;
  _stopAtMs = millis() + boundedDuration;
}

DriveMode ServoDrive::driveMode() const {
  return _mode;
}
