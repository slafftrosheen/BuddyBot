#include "ImuMonitor.h"

#include <M5Unified.h>

#include "Config.h"

void ImuMonitor::begin() {
  _reading = {};
  _reading.available = M5.Imu.isEnabled();
  _lastSampleMs = 0;
}

void ImuMonitor::update(uint32_t nowMs) {
  _reading.available = M5.Imu.isEnabled();
  if (!_reading.available) {
    _reading.valid = false;
    return;
  }

  if (nowMs - _lastSampleMs < IMU_SAMPLE_INTERVAL_MS) {
    return;
  }
  _lastSampleMs = nowMs;

  M5.Imu.update();
  const bool accelValid = M5.Imu.getAccel(
    &_reading.accelXG, &_reading.accelYG, &_reading.accelZG);
  const bool gyroValid = M5.Imu.getGyro(
    &_reading.gyroXDps, &_reading.gyroYDps, &_reading.gyroZDps);

  _reading.valid = accelValid && gyroValid;
  if (_reading.valid) {
    _reading.sampleTimeMs = nowMs;
  }
}
