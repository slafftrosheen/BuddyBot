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

class BootDiagnostics {
public:
  void begin(RobotAPI* robot);
  void update();
  
  BootPhase currentPhase() const { return _phase; }
  bool isComplete() const { return _phase == BootPhase::COMPLETE || _phase == BootPhase::DEGRADED || _phase == BootPhase::FAILED; }
  bool hasFailed() const { return _phase == BootPhase::FAILED; }
  
  const char* phaseName(BootPhase phase) const;

private:
  RobotAPI* _robot = nullptr;
  BootPhase _phase = BootPhase::NOT_STARTED;
  uint32_t _phaseStartTime = 0;
  
  void advanceTo(BootPhase nextPhase);
};
