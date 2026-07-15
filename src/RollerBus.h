#pragma once
#include <Arduino.h>
#include <Wire.h>

class RollerBus {
public:
  explicit RollerBus(uint8_t addr) : _addr(addr) {}

  bool begin() {
    Wire.beginTransmission(_addr);
    _connected = (Wire.endTransmission() == 0);
    return _connected;
  }

  bool isConnected() const { return _connected; }
  uint8_t address() const { return _addr; }

  bool setSpeed(int16_t speed) {
    if (!_connected) return false;
    // TODO: replace with official M5 Roller library call.
    return true;
  }

  bool setPosition(int32_t pos) {
    if (!_connected) return false;
    // TODO: replace with official M5 Roller library call.
    return true;
  }

  bool stop() {
    if (!_connected) return false;
    // TODO: replace with official M5 Roller library call.
    return true;
  }

private:
  uint8_t _addr;
  bool _connected = false;
};
