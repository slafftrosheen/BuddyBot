#include "ServoDiagnostics.h"
#include <Arduino.h>

#include "BuildProfiles.h"

void ServoDiagnostics::begin(RobotHal* hal) {
  _hal = hal;
}

void ServoDiagnostics::update(uint32_t nowMs) {
  if (_unlocked && (nowMs - _unlockedMs > 30000)) {
    // Timeout unlock
    lock();
  }
}

bool ServoDiagnostics::isUnlocked() const {
  return _unlocked;
}

void ServoDiagnostics::unlock() {
  _unlocked = true;
  _unlockedMs = millis();
}

void ServoDiagnostics::lock() {
  _unlocked = false;
  stopTest();
}

bool ServoDiagnostics::testWheel(ServoRole wheelRole, int16_t speed) {
  if (!_unlocked || !_hal) return false;
  
  if (_testingWheel && _testRole != wheelRole) {
    stopTest();
  }
  
  _unlockedMs = millis(); // extend timeout
  
  // Actually driving a single wheel requires bypassing IDrive entirely if IDrive doesn't support it.
  // IDrive doesn't have a single-wheel test. So we use the Servo8Bus directly if we are in servo mode.
  if (_hal->buildConfig().driveType == DriveControllerType::SERVO8_CONTINUOUS) {
    const ServoChannelConfig* activeConfig = getActiveServoConfig();
    if (activeConfig) {
      for (int i = 0; i < 8; i++) {
        if (activeConfig[i].enabled && activeConfig[i].role == wheelRole && activeConfig[i].continuousRotation) {
          int16_t out = activeConfig[i].continuous.stopUs;
          if (speed != 0) {
            int16_t offset = (speed * (activeConfig[i].continuous.forwardUs - activeConfig[i].continuous.stopUs)) / 100;
            if (activeConfig[i].continuous.direction == ServoDirection::INVERTED) offset = -offset;
            out += offset;
          }
          _hal->writeRawPulse(wheelRole, out);
          _testingWheel = true;
          _testRole = wheelRole;
          return true;
        }
      }
    }
  }
  
  return false;
}

void ServoDiagnostics::stopTest() {
  _testingWheel = false;
}
