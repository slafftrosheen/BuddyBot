#pragma once
#include <Arduino.h>
#include "Types.h"

struct DriveCommand {
  DriveMode mode;
  int16_t linear;
  int16_t angular;
  uint16_t durationMs;

  DriveCommand() = default;
  DriveCommand(DriveMode m, int16_t lin, int16_t ang, uint16_t dur)
  : mode(m), linear(lin), angular(ang), durationMs(dur) {}
};

struct JointLimits {
  int16_t minValue;
  int16_t maxValue;
  int16_t restValue;

  JointLimits() = default;
  JointLimits(int16_t minV, int16_t maxV, int16_t restV)
  : minValue(minV), maxValue(maxV), restValue(restV) {}
};

struct RangeReading {
  bool valid = false;
  uint16_t distanceMm = 0;
  uint32_t timestampMs = 0;
};
