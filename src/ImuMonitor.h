#pragma once

#include <Arduino.h>

struct ImuReading {
  bool available = false;
  bool valid = false;
  uint32_t sampleTimeMs = 0;
  float accelXG = 0.0f;
  float accelYG = 0.0f;
  float accelZG = 0.0f;
  float gyroXDps = 0.0f;
  float gyroYDps = 0.0f;
  float gyroZDps = 0.0f;
};

class ImuMonitor {
 public:
  void begin();
  void update(uint32_t nowMs);
  const ImuReading& reading() const { return _reading; }

 private:
  ImuReading _reading;
  uint32_t _lastSampleMs = 0;
};
