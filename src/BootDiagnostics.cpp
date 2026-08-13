#include "BootDiagnostics.h"
#include <Wire.h>

void BootDiagnostics::begin(RobotAPI* robot) {
  _robot = robot;
  _status = {};
  advanceTo(BootPhase::DISPLAY_OK);
}

void BootDiagnostics::advanceTo(BootPhase nextPhase) {
  _phase = nextPhase;
  _status.phase = nextPhase;
  _phaseStartTime = millis();
  Serial.printf("BOOT: -> %s\n", phaseName(_phase));
}

void BootDiagnostics::update() {
  if (isComplete()) return;
  
  uint32_t elapsed = millis() - _phaseStartTime;
  
  switch (_phase) {
    case BootPhase::NOT_STARTED:
      break;
      
    case BootPhase::DISPLAY_OK:
      _status.displayReady = true;
      if (elapsed > 200) {
        advanceTo(BootPhase::I2C_SCAN);
      }
      break;
      
    case BootPhase::I2C_SCAN: {
      bool foundServo = false;
      bool foundSonic = false;
      Wire1.beginTransmission(0x25);
      if (Wire1.endTransmission() == 0) foundServo = true;
      
      Wire1.beginTransmission(0x57);
      if (Wire1.endTransmission() == 0) foundSonic = true;

      _status.servoBusPresent = foundServo;
      _status.sonicPresent = foundSonic;
      
      Serial.printf("BOOT: I2C Scan - Servo8: %s, Sonic: %s\n", foundServo ? "OK" : "MISSING", foundSonic ? "OK" : "MISSING");
      
      advanceTo(BootPhase::SERVO8_CHECK);
      break;
    }
      
    case BootPhase::SERVO8_CHECK:
      if (elapsed > 100) {
        if (!_robot->driveAvailable() && _robot->fourWheelDriveConfigured()) {
          Serial.println("BOOT: WARNING - Servo8 missing for 4WD config");
        }
        advanceTo(BootPhase::SONIC_CHECK);
      }
      break;
      
    case BootPhase::SONIC_CHECK:
      if (elapsed > 100) {
        advanceTo(BootPhase::DRIVE_SAFE_STOP);
      }
      break;
      
    case BootPhase::DRIVE_SAFE_STOP:
      if (elapsed > 100) {
        _robot->stopAll();
        _status.driveStopped = !_robot->isArmed();
        advanceTo(BootPhase::MANIPULATOR_CHECK);
      }
      break;
      
    case BootPhase::MANIPULATOR_CHECK:
      if (elapsed > 100) {
        _status.manipulatorsChecked = true;
        advanceTo(BootPhase::COMPLETE);
      }
      break;
      
    default:
      break;
  }
}

const char* BootDiagnostics::phaseName(BootPhase phase) const {
  switch (phase) {
    case BootPhase::NOT_STARTED: return "NOT_STARTED";
    case BootPhase::DISPLAY_OK: return "DISPLAY_OK";
    case BootPhase::I2C_SCAN: return "I2C_SCAN";
    case BootPhase::SERVO8_CHECK: return "SERVO8_CHECK";
    case BootPhase::SONIC_CHECK: return "SONIC_CHECK";
    case BootPhase::DRIVE_SAFE_STOP: return "DRIVE_SAFE_STOP";
    case BootPhase::MANIPULATOR_CHECK: return "MANIPULATOR_CHECK";
    case BootPhase::COMPLETE: return "COMPLETE";
    case BootPhase::DEGRADED: return "DEGRADED";
    case BootPhase::FAILED: return "FAILED";
    default: return "UNKNOWN";
  }
}
