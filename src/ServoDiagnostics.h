#pragma once
#include <stdint.h>
#include "RobotHal.h"

enum class ServoDiagnosticResult : uint8_t {
  IDLE,
  LOCKED,
  BENCH_DISABLED,
  DRIVE_ARMED,
  INVALID_ROLE,
  INVALID_PULSE,
  BUS_FAILURE,
  ACTIVE,
  STOPPED,
  TIMED_OUT
};

class ServoDiagnostics {
public:
  ServoDiagnostics() = default;
  void begin(RobotHal* hal);
  
  void update(uint32_t nowMs);
  
  bool isUnlocked() const;
  bool unlock();
  void lock();
  
  // Single wheel diagnostics
  bool testWheel(ServoRole wheelRole, int16_t pulseUs);
  void stopTest();
  ServoDiagnosticResult lastResult() const;
  const char* lastResultName() const;

private:
  RobotHal* _hal;
  bool _unlocked = false;
  uint32_t _unlockedMs = 0;
  bool _testingWheel = false;
  ServoRole _testRole = ServoRole::UNUSED;
  ServoDiagnosticResult _lastResult = ServoDiagnosticResult::IDLE;
};
