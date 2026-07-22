#pragma once
#include <stdint.h>
#include "RobotHal.h"

class ServoDiagnostics {
public:
  ServoDiagnostics() = default;
  void begin(RobotHal* hal);
  
  void update(uint32_t nowMs);
  
  bool isUnlocked() const;
  void unlock();
  void lock();
  
  // Single wheel diagnostics
  bool testWheel(ServoRole wheelRole, int16_t speed);
  void stopTest();

private:
  RobotHal* _hal;
  bool _unlocked = false;
  uint32_t _unlockedMs = 0;
  bool _testingWheel = false;
  ServoRole _testRole;
};
