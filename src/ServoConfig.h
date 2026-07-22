#pragma once

#include <Arduino.h>

enum class ServoRole : uint8_t {
  UNUSED,
  DRIVE_FRONT_LEFT,
  DRIVE_REAR_LEFT,
  DRIVE_FRONT_RIGHT,
  DRIVE_REAR_RIGHT,
  HEAD,
  LEFT_ARM,
  RIGHT_ARM,
  ACCESSORY_1
};

enum class ServoDirection : int8_t {
  NORMAL = 1,
  INVERTED = -1
};

struct ContinuousServoCalibration {
  uint16_t stopUs;
  uint16_t forwardUs;
  uint16_t reverseUs;
  uint16_t minimumUs;
  uint16_t maximumUs;
  uint16_t trimUs;
  ServoDirection direction;
};

struct PositionalServoCalibration {
  uint8_t minimumDeg;
  uint8_t maximumDeg;
  uint8_t restDeg;
  bool inverted;
  uint16_t minimumPulseUs;
  uint16_t maximumPulseUs;
};

struct ServoChannelConfig {
  uint8_t channel;
  ServoRole role;
  bool enabled;
  bool continuousRotation;
  ContinuousServoCalibration continuous;
  PositionalServoCalibration positional;
};

// Size matching SERVO8_CHANNEL_COUNT from Config.h (8)
extern const ServoChannelConfig DEFAULT_4WD_SERVO_CONFIG[8];
extern const ServoChannelConfig LEGACY_2WD_SERVO_CONFIG[8];
