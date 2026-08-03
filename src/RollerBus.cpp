#include "RollerBus.h"
#include "Config.h"

// M5 Roller485 I2C Registers
static constexpr uint8_t REG_MOTOR_ENABLE = 0x00;
static constexpr uint8_t REG_MODE_SETTING = 0x01;
static constexpr uint8_t REG_SPEED_CONTROL = 0x40;

static constexpr uint8_t MODE_SPEED = 1;

RollerBus::RollerBus(TwoWire& wire, uint8_t address)
  : _wire(wire), _addr(address) {}

bool RollerBus::writeRegister(uint8_t reg, uint8_t value) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  _wire.write(value);
  const bool success = _wire.endTransmission() == 0;
  if (!success) {
    _connected = false;
  }
  return success;
}

bool RollerBus::writeRegister32(uint8_t reg, int32_t value) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  _wire.write(value & 0xFF);
  _wire.write((value >> 8) & 0xFF);
  _wire.write((value >> 16) & 0xFF);
  _wire.write((value >> 24) & 0xFF);
  const bool success = _wire.endTransmission() == 0;
  if (!success) {
    _connected = false;
  }
  return success;
}

bool RollerBus::begin() {
  _connected = false;

  _wire.beginTransmission(_addr);
  if (_wire.endTransmission() != 0) {
    return false;
  }

  // Force zero target speed
  if (!setSpeedRpm(0)) return false;

  // Set mode to Speed (1)
  if (!writeRegister(REG_MODE_SETTING, MODE_SPEED)) return false;

  // Disable motor output
  if (!writeRegister(REG_MOTOR_ENABLE, 0)) return false;

  _connected = true;
  return true;
}

bool RollerBus::isConnected() const {
  return _connected;
}

uint8_t RollerBus::address() const {
  return _addr;
}

bool RollerBus::arm() {
  if (!_connected) return false;

  // Ensure speed mode is selected
  if (!writeRegister(REG_MODE_SETTING, MODE_SPEED)) return false;

  // Enable motor output
  return writeRegister(REG_MOTOR_ENABLE, 1);
}

bool RollerBus::disarm() {
  if (!_connected) return false;
  
  // Send zero speed first
  setSpeedRpm(0);
  
  // Disable motor output
  return writeRegister(REG_MOTOR_ENABLE, 0);
}

bool RollerBus::setSpeedRpm(int16_t rpm) {
  rpm = constrain(rpm, -ROLLER_MAX_RPM, ROLLER_MAX_RPM);

  // Value = Actual Speed * 100
  int32_t val = (int32_t)rpm * 100;
  if (writeRegister32(REG_SPEED_CONTROL, val)) {
    _lastSpeedRpm = rpm;
    return true;
  }
  _lastSpeedRpm = 0;
  return false;
}

bool RollerBus::setPositionCounts(int32_t counts) {
  (void)counts;
  return false; // Unsupported in this configuration
}

bool RollerBus::stop() {
  return setSpeedRpm(0);
}

int16_t RollerBus::lastSpeedRpm() const {
  return _lastSpeedRpm;
}
