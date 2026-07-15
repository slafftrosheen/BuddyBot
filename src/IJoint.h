#pragma once
#include <Arduino.h>

class IJoint {
public:
  virtual ~IJoint() = default;
  virtual bool begin() = 0;
  virtual bool isConnected() const = 0;
  virtual void moveTo(int16_t value) = 0;
  virtual int16_t current() const = 0;
  virtual void rest() = 0;
};
