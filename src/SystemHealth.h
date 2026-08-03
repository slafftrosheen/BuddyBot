#pragma once

#include <stdint.h>

class SystemHealth {
 public:
  void begin();
  void feed();

  bool watchdogActive() const;
  uint32_t resetReason() const;
  const char* resetReasonName() const;

 private:
  bool _watchdogActive = false;
  uint32_t _resetReason = 0;
};
