#pragma once
#include "HalTypes.h"

enum class RangeSensorHealth : uint8_t {
  UNINITIALIZED,
  READY,
  STALE,
  INVALID,
  UNAVAILABLE
};

class IRangeSensor {
public:
  virtual ~IRangeSensor() = default;
  virtual bool begin() = 0;
  virtual bool isConnected() const = 0;
  virtual void update() = 0;
  virtual RangeReading reading() const = 0;
  virtual RangeSensorHealth health() const = 0;
  virtual uint16_t consecutiveInvalidSamples() const = 0;
};
