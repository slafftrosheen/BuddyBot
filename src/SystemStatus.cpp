#include "SystemStatus.h"
#include "BuildProfiles.h"
#include "SystemHealth.h"

FirmwareIdentity getFirmwareIdentity() {
  return FirmwareIdentity{
    FIRMWARE_NAME,
    FIRMWARE_VERSION,
    FIRMWARE_CHANNEL,
    getActiveBuildName(),
    __DATE__,
    __TIME__
  };
}

void SystemStatus::begin(
  RobotAPI* robot,
  RobotHal* hal,
  BootDiagnostics* diag,
  SystemHealth* health
) {
  _robot = robot;
  _hal = hal;
  _diag = diag;
  _health = health;
}

void SystemStatus::printStatus() const {
  const HardwareManifest manifest = getActiveHardwareManifest();
  Serial.printf("profile=%s\n", manifest.profileName);
  Serial.printf("hardware_manifest_version=%u\n", manifest.schemaVersion);
  Serial.printf("grove_i2c_sda=%d\n", I2C_SDA_PIN);
  Serial.printf("grove_i2c_scl=%d\n", I2C_SCL_PIN);

  if (_hal && _hal->drive()) {
    Serial.printf("drive_armed=%s\n", _hal->drive()->isArmed() ? "true" : "false");
    Serial.printf("drive_mode=%u\n", unsigned(_hal->drive()->driveMode()));
  } else {
    Serial.println("drive=none");
  }

  if (_robot) {
    ActuatorCapabilities caps = _robot->actuatorCapabilities();
    Serial.printf("drive_4wd=%s\n", caps.fourWheelDrive ? "true" : "false");
    
    for (uint8_t i = 0; i < (uint8_t)ManipulatorId::COUNT; i++) {
      ManipulatorState s = _robot->manipulatorState((ManipulatorId)i);
      if (s.available) {
        Serial.printf("manipulator_%u_deg=%d\n", i, s.currentDeg);
        Serial.printf("manipulator_%u_moving=%s\n", i, s.moving ? "true" : "false");
      }
    }
  }

  RangeReading r = _robot ? _robot->rangeReading() : RangeReading{};
  if (r.valid) {
    Serial.printf("range_mm=%u\n", r.distanceMm);
  } else {
    Serial.println("range_mm=invalid");
  }

  Serial.printf("obstacle=%s\n", (_robot && _robot->obstacleDetected()) ? "true" : "false");
  Serial.printf("mood=%u\n", _robot ? unsigned(_robot->getMood()) : 0);
  if (_robot) {
    const ImuReading& imu = _robot->imuReading();
    Serial.printf("imu_available=%s\n", imu.available ? "true" : "false");
    Serial.printf("imu_valid=%s\n", imu.valid ? "true" : "false");
    Serial.printf("safety_state=%s\n", SafetySupervisor::stateName(_robot->safetyState()));
    Serial.printf("safety_fault=%s\n", SafetySupervisor::faultName(_robot->safetyFault()));
    Serial.printf("safety_state_changed_ms=%u\n", _robot->safetyStateChangedAtMs());
    if (_robot->diagnostics()) {
      Serial.printf("servo_diagnostic_unlocked=%s\n", _robot->diagnostics()->isUnlocked() ? "true" : "false");
      Serial.printf("servo_diagnostic_result=%s\n", _robot->diagnostics()->lastResultName());
    }
  }
  
  Serial.printf("wifi_running=%s\n", _wifiRunning ? "true" : "false");
  if (_wifiRunning) {
    Serial.printf("wifi_ssid=%s\n", _wifiSsid);
    Serial.printf("wifi_ip=%s\n", _wifiIp);
  }
  Serial.printf("wifi_clients=%u\n", _wifiClients);
  Serial.printf("wifi_controller=%s\n", _wifiHasController ? "true" : "false");
  Serial.printf("wifi_pairing_available=%s\n", _wifiPairingAvailable ? "true" : "false");
  
  if (_diag) {
    Serial.printf("boot_diag_complete=%s\n", _diag->isComplete() ? "true" : "false");
    Serial.printf("boot_diag_phase=%s\n", _diag->phaseName(_diag->currentPhase()));
    const BootDiagnosticStatus& boot = _diag->status();
    Serial.printf("boot_display=%s\n", boot.displayReady ? "pass" : "pending");
    Serial.printf("boot_servo_bus=%s\n", boot.servoBusPresent ? "pass" : "missing");
    Serial.printf("boot_sonic=%s\n", boot.sonicPresent ? "pass" : "missing");
    Serial.printf("boot_drive_stopped=%s\n", boot.driveStopped ? "pass" : "pending");
    Serial.printf("boot_manipulators=%s\n", boot.manipulatorsChecked ? "pass" : "pending");
  }

  if (_health) {
    Serial.printf("task_watchdog=%s\n", _health->watchdogActive() ? "active" : "inactive");
    Serial.printf("reset_reason=%s\n", _health->resetReasonName());
  }
  Serial.printf("heap_free_bytes=%u\n", ESP.getFreeHeap());
}

void SystemStatus::printEvents() const {
  EventLogEntry entries[EventLog::CAPACITY];
  size_t count = EventLog::instance().getEntries(entries, EventLog::CAPACITY);
  Serial.printf("events_count=%u\n", count);
  for (size_t i = 0; i < count; i++) {
    Serial.printf(
      "event[%u] ts=%u sev=%s component=%s code=%s correlation=%u\n",
      i,
      entries[i].timestampMs,
      entries[i].severity,
      entries[i].component,
      entries[i].code,
      entries[i].correlationId
    );
  }
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

void SystemStatus::setWifiStatus(bool running, const char* ssid, const char* ip, uint8_t clients, bool hasController, bool pairingAvailable) {
  _wifiRunning = running;
  if (ssid) {
    strncpy(_wifiSsid, ssid, sizeof(_wifiSsid) - 1);
    _wifiSsid[sizeof(_wifiSsid) - 1] = '\0';
  } else {
    _wifiSsid[0] = '\0';
  }
  if (ip) {
    strncpy(_wifiIp, ip, sizeof(_wifiIp) - 1);
    _wifiIp[sizeof(_wifiIp) - 1] = '\0';
  } else {
    _wifiIp[0] = '\0';
  }
  _wifiClients = clients;
  _wifiHasController = hasController;
  _wifiPairingAvailable = pairingAvailable;
}
