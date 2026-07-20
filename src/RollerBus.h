#pragma once
#include <Arduino.h>
#include <Wire.h>

class RollerBus {
public:
  explicit RollerBus(TwoWire& wire, uint8_t address);

  bool begin();
  bool isConnected() const;
  uint8_t address() const;

  bool arm();
  bool disarm();
  bool setSpeedRpm(int16_t rpm);
  bool setPositionCounts(int32_t counts);
  bool stop();
  int16_t lastSpeedRpm() const;

private:
  TwoWire& _wire;
  uint8_t _addr;
  bool _connected = false;
  int16_t _lastSpeedRpm = 0;

  bool writeRegister(uint8_t reg, uint8_t value);
  bool writeRegister32(uint8_t reg, int32_t value);
};
