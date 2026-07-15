#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "IRangeSensor.h"
#include "Config.h"

class SonicRangeSensor : public IRangeSensor {
public:
  bool begin() override;
  bool isConnected() const override;
  void update() override;
  RangeReading reading() const override;

private:
  bool _connected = false;
  RangeReading _reading;
};
