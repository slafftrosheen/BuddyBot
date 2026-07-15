#include "SonicRangeSensor.h"

bool SonicRangeSensor::begin() {
  Wire.beginTransmission(SONIC_I2C_ADDR);
  _connected = (Wire.endTransmission() == 0);
  _reading = {};
  return _connected;
}

bool SonicRangeSensor::isConnected() const {
  return _connected;
}

void SonicRangeSensor::update() {
  if (!_connected) {
    return;
  }

  // Placeholder transport layer.
  // Replace with the official M5Unit-Sonic library once integrated.
  _reading.valid = false;
  _reading.distanceMm = 0;
  _reading.timestampMs = millis();
}

RangeReading SonicRangeSensor::reading() const {
  return _reading;
}
