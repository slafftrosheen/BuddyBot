#include "SystemStatus.h"
#include "BuildProfiles.h"

void SystemStatus::begin(RobotAPI* robot, RobotHal* hal) {
  _robot = robot;
  _hal = hal;
}

void SystemStatus::printStatus() const {
  Serial.printf("profile=%s\n", getActiveBuildName());
  Serial.printf("grove_i2c_sda=%d\n", I2C_SDA_PIN);
  Serial.printf("grove_i2c_scl=%d\n", I2C_SCL_PIN);

  if (_hal && _hal->drive()) {
    Serial.printf("drive_armed=%s\n", _hal->drive()->isArmed() ? "true" : "false");
    Serial.printf("drive_mode=%u\n", unsigned(_hal->drive()->driveMode()));
  } else {
    Serial.println("drive=none");
  }

  RangeReading r = _robot ? _robot->rangeReading() : RangeReading{};
  if (r.valid) {
    Serial.printf("range_mm=%u\n", r.distanceMm);
  } else {
    Serial.println("range_mm=invalid");
  }

  Serial.printf("obstacle=%s\n", (_robot && _robot->obstacleDetected()) ? "true" : "false");
  Serial.printf("mood=%u\n", _robot ? unsigned(_robot->getMood()) : 0);
}
