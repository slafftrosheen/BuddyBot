#pragma once
#include <stdint.h>
#include "JointMotion.h"

enum class ManipulatorId : uint8_t {
  HEAD,
  LEFT_ARM,
  RIGHT_ARM,
  ACCESSORY_1,
  COUNT
};

struct ManipulatorState {
  bool available;
  int16_t currentDeg;
  int16_t targetDeg;
  bool moving;
  JointMotionState motionState;
};

struct ActuatorCapabilities {
  bool servoBusConnected;
  bool driveAvailable;
  bool fourWheelDrive;
  uint8_t driveServoCount;
  bool manipulatorAvailable;
  uint8_t manipulatorCount;
  bool headAvailable;
  bool leftArmAvailable;
  bool rightArmAvailable;
  bool accessory1Available;
};
