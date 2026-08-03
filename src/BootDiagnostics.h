#pragma once
#include <Arduino.h>
#include "RobotAPI.h"

enum class BootPhase : uint8_t {
  NOT_STARTED,
  DISPLAY_OK,
  I2C_SCAN,
  SERVO8_CHECK,
  SONIC_CHECK,
  DRIVE_SAFE_STOP,
  MANIPULATOR_CHECK,
  COMPLETE,
  DEGRADED,
  FAILED
};

struct BootDiagnosticStatus {
  BootPhase phase = BootPhase::NOT_STARTED;
  bool displayReady = false;
  bool servoBusPresent = false;
  bool sonicPresent = false;
  bool driveStopped = false;
  bool manipulatorsChecked = false;
};

class BootDiagnostics {
public:
  void begin(RobotAPI* robot);
  void update();
  
  BootPhase currentPhase() const { return _phase; }
  bool isComplete() const { return _phase == BootPhase::COMPLETE || _phase == BootPhase::DEGRADED || _phase == BootPhase::FAILED; }
  bool hasFailed() const { return _phase == BootPhase::FAILED; }
  
  const char* phaseName(BootPhase phase) const;
  const BootDiagnosticStatus& status() const { return _status; }

private:
  RobotAPI* _robot = nullptr;
  BootPhase _phase = BootPhase::NOT_STARTED;
  uint32_t _phaseStartTime = 0;
  BootDiagnosticStatus _status;
  
  void advanceTo(BootPhase nextPhase);
};
