#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <M5_UNIT_8SERVO.h>
#include "Config.h"

class Servo8Bus {
public:
  bool begin() {
    _connected = _unit.begin(&Wire, I2C_SDA_PIN, I2C_SCL_PIN, SERVOS8_ADDR);
    if (_connected) {
      _unit.setAllPinMode(SERVO_CTL_MODE);
    }
    return _connected;
  }

  bool isConnected() const { return _connected; }

  void setPulse(uint8_t ch, uint16_t us) {
    if (!_connected || ch > 7) return;
    _unit.setServoPulse(ch, constrain(us, SERVO_MIN_US, SERVO_MAX_US));
  }

  void setAngle(uint8_t ch, uint8_t deg) {
    if (!_connected || ch > 7) return;
    _unit.setServoAngle(ch, constrain(deg, 0, 180));
  }

private:
  M5_UNIT_8SERVO _unit;
  bool _connected = false;
};
