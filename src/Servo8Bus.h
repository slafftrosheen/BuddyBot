#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <M5_UNIT_8SERVO.h>
#include "Config.h"

#include "ServoConfig.h"

class Servo8Bus {
public:
  bool begin();
  bool isConnected() const;

  bool configureChannel(const ServoChannelConfig& config);
  bool configureAll(const ServoChannelConfig configs[8]);

  bool writePulse(uint8_t channel, uint16_t pulseUs);
  bool writeAngle(uint8_t channel, uint8_t degrees);

  bool stopContinuous(uint8_t channel);
  void stopAllContinuous();

  bool hasRole(ServoRole role) const;
  const ServoChannelConfig* configForChannel(uint8_t channel) const;
  const ServoChannelConfig* configForRole(ServoRole role) const;

  bool channelIsContinuous(uint8_t channel) const;
  bool channelIsPositional(uint8_t channel) const;

  uint16_t lastPulse(uint8_t channel) const;
  uint8_t lastAngle(uint8_t channel) const;

private:
  M5_UNIT_8SERVO _unit;
  bool _connected = false;
  ServoChannelConfig _channels[SERVO8_CHANNEL_COUNT];
  uint16_t _lastPulse[SERVO8_CHANNEL_COUNT] = {0};
  uint8_t _lastAngle[SERVO8_CHANNEL_COUNT] = {0};
};
