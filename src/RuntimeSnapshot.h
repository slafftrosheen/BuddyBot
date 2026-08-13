#pragma once

#include <stdint.h>

struct RuntimeSafetySnapshot {
  uint8_t state;
  uint8_t fault;

  bool bootComplete;
  bool armed;
  bool estopped;
  bool faulted;

  bool autonomyEnabled;
  bool autonomyMotionAllowed;

  uint32_t stateChangedAtMs;
  uint32_t manualOverrideUntilMs;
};

struct RuntimeDriveSnapshot {
  bool available;
  bool active;
  bool forward;
  bool blocked;

  uint8_t mode;

  uint32_t lastCommandAtMs;
};

struct RuntimeRangeSnapshot {
  bool available;
  bool valid;
  uint16_t distanceMm;

  uint8_t health;

  uint32_t sampleTimeMs;
  uint32_t consecutiveInvalid;
};

struct RuntimeImuSnapshot {
  bool available;
  bool valid;

  uint32_t sampleTimeMs;

  float accelXG;
  float accelYG;
  float accelZG;

  float gyroXDps;
  float gyroYDps;
  float gyroZDps;
};

struct RuntimeBehaviorSnapshot {
  bool actionRunning;
  uint8_t action;

  uint8_t mood;
  uint8_t persona;

  bool autonomyEnabled;
};

struct RuntimeHardwareSnapshot {
  bool driveAvailable;
  bool rangeAvailable;
  bool imuAvailable;

  bool servoBusPresent;
  bool sonicPresent;
};

struct RuntimeSnapshot {
  uint32_t capturedAtMs;

  RuntimeSafetySnapshot safety;
  RuntimeDriveSnapshot drive;
  RuntimeRangeSnapshot range;
  RuntimeImuSnapshot imu;
  RuntimeBehaviorSnapshot behavior;
  RuntimeHardwareSnapshot hardware;
};

class SafetySupervisor;
RuntimeSafetySnapshot buildSafetySnapshot(const SafetySupervisor* supervisor, uint32_t nowMs);
