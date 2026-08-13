#include "SonicRangeSensor.h"

#include <math.h>

uint16_t SonicRangeSensor::getMedian(uint16_t a, uint16_t b, uint16_t c) const {
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}

bool SonicRangeSensor::begin() {
  _health = RangeSensorHealth::UNINITIALIZED;
  
  // Initialize the sensor via the existing Wire bus.
<<<<<<< HEAD
  _sensor.begin(&Wire1, 9, 10, SONIC_I2C_ADDR);
=======
  _sensor.begin(&Wire, I2C_SDA_PIN, I2C_SCL_PIN, SONIC_I2C_ADDR);
>>>>>>> 638c121ac325cddcc76382fcae5f99ddbbccb279

  // Verify connection by doing a simple I2C probe
  Wire1.beginTransmission(SONIC_I2C_ADDR);
  _connected = (Wire1.endTransmission() == 0);
  
  if (_connected) {
    _health = RangeSensorHealth::READY;
  } else {
    _health = RangeSensorHealth::UNAVAILABLE;
  }
  
  _reading = {};
  _lastAttemptMs = 0;
  _lastSuccessfulSampleMs = 0;
  _consecutiveInvalid = 0;
  _filterIndex = 0;
  _filterFull = false;

  return _connected;
}

bool SonicRangeSensor::isConnected() const {
  return _connected;
}

RangeSensorHealth SonicRangeSensor::health() const {
  return _health;
}

uint16_t SonicRangeSensor::consecutiveInvalidSamples() const {
  return _consecutiveInvalid;
}

void SonicRangeSensor::update() {
  if (!_connected) {
    _health = RangeSensorHealth::UNAVAILABLE;
    return;
  }

  uint32_t now = millis();
  
  if (_health == RangeSensorHealth::READY && _reading.valid) {
    if (now - _reading.sampleTimeMs > SONIC_STALE_AFTER_MS) {
      _reading.valid = false;
      _health = RangeSensorHealth::STALE;
    }
  }

  if (now - _lastAttemptMs < SONIC_SAMPLE_INTERVAL_MS) {
    return; // Rate limiting
  }

  _lastAttemptMs = now;
  float distFloat = _sensor.getDistance();
  const bool finiteDistance = isfinite(distFloat);
  uint16_t rawDist = finiteDistance && distFloat > 0.0f && distFloat <= 65535.0f
    ? static_cast<uint16_t>(distFloat)
    : 0;
  
  bool sampleValid = finiteDistance;
  if (!sampleValid || rawDist < SONIC_MIN_VALID_MM || rawDist > SONIC_MAX_VALID_MM) {
    sampleValid = false;
  } else if (_reading.valid) {
    // Check for unrealistic spike
    uint16_t diff = (rawDist > _reading.distanceMm) ? (rawDist - _reading.distanceMm) : (_reading.distanceMm - rawDist);
    if (diff > SONIC_MAX_STEP_MM) {
      sampleValid = false;
    }
  }

  if (sampleValid) {
    _lastSuccessfulSampleMs = now;
    _consecutiveInvalid = 0;
    
    _filterBuffer[_filterIndex] = rawDist;
    _filterIndex++;
    if (_filterIndex >= 3) {
      _filterIndex = 0;
      _filterFull = true;
    }

    if (_filterFull) {
      _reading.distanceMm = getMedian(_filterBuffer[0], _filterBuffer[1], _filterBuffer[2]);
    } else {
      _reading.distanceMm = rawDist;
    }

    _reading.valid = true;
    _reading.sampleTimeMs = now;
    _health = RangeSensorHealth::READY;
  } else {
    _consecutiveInvalid++;
    _reading.valid = false;
    if (_consecutiveInvalid >= SONIC_INVALID_SAMPLES_TO_FAULT) {
      _health = RangeSensorHealth::INVALID;
    } else {
      _health = RangeSensorHealth::STALE;
    }
  }
}

RangeReading SonicRangeSensor::reading() const {
  return _reading;
}
