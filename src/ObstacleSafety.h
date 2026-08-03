#pragma once
#include <Arduino.h>
#include "HalTypes.h"
#include "Types.h"
#include "Config.h"

enum class ObstacleSafetyState : uint8_t {
  CLEAR,
  CAUTION,
  BLOCKED,
  SENSOR_UNAVAILABLE
};

enum class SafetyStopReason : uint8_t {
  NONE,
  OBSTACLE_BLOCKED,
  RANGE_SENSOR_STALE,
  RANGE_SENSOR_INVALID,
  AUTONOMY_TIMEOUT,
  MANUAL_STOP,
  CONTROLLER_DISCONNECT,
  CONTROLLER_LEASE_EXPIRED,
  DRIVE_WATCHDOG,
  PHYSICAL_ESTOP,
  IMU_UNAVAILABLE,
  IMU_INVALID,
  DRIVE_UNAVAILABLE,
  DISARMED
};

struct ObstacleSafetyStatus {
  ObstacleSafetyState state;
  SafetyStopReason lastStopReason;
  uint32_t lastStopMs;
  uint16_t filteredRangeMm;
  bool rangeValid;
  bool forwardMotionBlocked;
};

class ObstacleSafety {
public:
  void begin();
  void update(const RangeReading& range, uint32_t nowMs);

  bool allowsDrive(DriveMode requested) const;
  bool blocksForwardMotion() const;
  ObstacleSafetyStatus status() const;

  void recordExternalStop(SafetyStopReason reason, uint32_t nowMs);
  void clearExternalStopIfSafe();

private:
  ObstacleSafetyState _state = ObstacleSafetyState::SENSOR_UNAVAILABLE;
  SafetyStopReason _lastStopReason = SafetyStopReason::NONE;
  uint32_t _lastStopMs = 0;
  uint16_t _rangeMm = 0;
  bool _rangeValid = false;
  uint32_t _clearSinceMs = 0;
};
