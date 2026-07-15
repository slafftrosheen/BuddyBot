#pragma once
#include "HalTypes.h"

class IRangeSensor {
public:
  virtual ~IRangeSensor() = default;
  virtual bool begin() = 0;
  virtual bool isConnected() const = 0;
  virtual void update() = 0;
  virtual RangeReading reading() const = 0;
};
