#include "Servo8Bus.h"

bool Servo8Bus::begin() {
  _connected = _unit.begin(&Wire, I2C_SDA_PIN, I2C_SCL_PIN, SERVOS8_ADDR);
  if (_connected) {
    _unit.setAllPinMode(SERVO_CTL_MODE);
    stopAllContinuous();
  }
  return _connected;
}

bool Servo8Bus::isConnected() const {
  return _connected;
}

bool Servo8Bus::configureChannel(const ServoChannelConfig& config) {
  if (config.channel >= SERVO8_CHANNEL_COUNT) return false;
  _channels[config.channel] = config;
  return true;
}

bool Servo8Bus::configureAll(const ServoChannelConfig configs[8]) {
  bool success = true;
  for (uint8_t i = 0; i < SERVO8_CHANNEL_COUNT; i++) {
    if (!configureChannel(configs[i])) success = false;
  }
  return success;
}

bool Servo8Bus::writePulse(uint8_t channel, uint16_t pulseUs) {
  if (!_connected || channel >= SERVO8_CHANNEL_COUNT) return false;
  
  const ServoChannelConfig& cfg = _channels[channel];
  if (!cfg.enabled || !cfg.continuousRotation) return false;

  uint16_t clamped = constrain(pulseUs, cfg.continuous.minimumUs, cfg.continuous.maximumUs);
  _unit.setServoPulse(channel, clamped);
  _lastPulse[channel] = clamped;
  return true;
}

bool Servo8Bus::writeAngle(uint8_t channel, uint8_t degrees) {
  if (!_connected || channel >= SERVO8_CHANNEL_COUNT) return false;
  
  const ServoChannelConfig& cfg = _channels[channel];
  if (!cfg.enabled || cfg.continuousRotation) return false;

  uint8_t clamped = constrain(degrees, cfg.positional.minimumDeg, cfg.positional.maximumDeg);
  _unit.setServoAngle(channel, clamped);
  _lastAngle[channel] = clamped;
  return true;
}

bool Servo8Bus::stopContinuous(uint8_t channel) {
  if (channel >= SERVO8_CHANNEL_COUNT) return false;
  const ServoChannelConfig& cfg = _channels[channel];
  if (!cfg.enabled || !cfg.continuousRotation) return false;
  
  return writePulse(channel, cfg.continuous.stopUs);
}

void Servo8Bus::stopAllContinuous() {
  for (uint8_t i = 0; i < SERVO8_CHANNEL_COUNT; i++) {
    const ServoChannelConfig& cfg = _channels[i];
    if (cfg.enabled && cfg.continuousRotation) {
      writePulse(i, cfg.continuous.stopUs);
    }
  }
}

bool Servo8Bus::hasRole(ServoRole role) const {
  return configForRole(role) != nullptr;
}

const ServoChannelConfig* Servo8Bus::configForChannel(uint8_t channel) const {
  if (channel >= SERVO8_CHANNEL_COUNT) return nullptr;
  return &_channels[channel];
}

const ServoChannelConfig* Servo8Bus::configForRole(ServoRole role) const {
  if (role == ServoRole::UNUSED) return nullptr;
  
  for (uint8_t i = 0; i < SERVO8_CHANNEL_COUNT; i++) {
    if (_channels[i].enabled && _channels[i].role == role) {
      return &_channels[i];
    }
  }
  return nullptr;
}

bool Servo8Bus::channelIsContinuous(uint8_t channel) const {
  const ServoChannelConfig* cfg = configForChannel(channel);
  return cfg && cfg->enabled && cfg->continuousRotation;
}

bool Servo8Bus::channelIsPositional(uint8_t channel) const {
  const ServoChannelConfig* cfg = configForChannel(channel);
  return cfg && cfg->enabled && !cfg->continuousRotation;
}

uint16_t Servo8Bus::lastPulse(uint8_t channel) const {
  if (channel >= SERVO8_CHANNEL_COUNT) return 0;
  return _lastPulse[channel];
}

uint8_t Servo8Bus::lastAngle(uint8_t channel) const {
  if (channel >= SERVO8_CHANNEL_COUNT) return 0;
  return _lastAngle[channel];
}
