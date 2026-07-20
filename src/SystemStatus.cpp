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
  
  Serial.printf("wifi_ssid=%s\n", _wifiSsid);
  Serial.printf("wifi_ip=%s\n", _wifiIp);
  Serial.printf("wifi_clients=%u\n", _wifiClients);
  Serial.printf("wifi_controller=%s\n", _wifiHasController ? "true" : "false");
}

void SystemStatus::setPairingCode(const char* code) {
  if (code) {
    strncpy(_pairingCode, code, sizeof(_pairingCode) - 1);
    _pairingCode[sizeof(_pairingCode) - 1] = '\0';
  }
}

void SystemStatus::clearPairingCode() {
  _pairingCode[0] = '\0';
}

void SystemStatus::setWifiStatus(const char* ssid, const char* ip, uint8_t clients, bool hasController) {
  if (ssid) strncpy(_wifiSsid, ssid, sizeof(_wifiSsid) - 1);
  if (ip) strncpy(_wifiIp, ip, sizeof(_wifiIp) - 1);
  _wifiClients = clients;
  _wifiHasController = hasController;
}
