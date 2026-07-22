#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "IRangeSensor.h"
#include "Config.h"
#include <Unit_Sonic.h>

class SonicRangeSensor : public IRangeSensor {
public:
  bool begin() override;
  bool isConnected() const override;
  void update() override;
  RangeReading reading() const override;

  RangeSensorHealth health() const override;
  uint16_t consecutiveInvalidSamples() const override;

private:
  SONIC_I2C _sensor;
  bool _connected = false;
  RangeReading _reading;
  RangeSensorHealth _health = RangeSensorHealth::UNINITIALIZED;
  
  uint32_t _lastAttemptMs = 0;
  uint32_t _lastSuccessfulSampleMs = 0;
  uint16_t _consecutiveInvalid = 0;
  
  uint16_t _filterBuffer[3] = {0};
  uint8_t _filterIndex = 0;
  bool _filterFull = false;

  uint16_t getMedian(uint16_t a, uint16_t b, uint16_t c) const;
};
