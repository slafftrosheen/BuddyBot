#pragma once

#include <stdint.h>

class RobotAPI;

enum class CapabilityReason : uint8_t {
    NONE = 0,

    NOT_IMPLEMENTED,
    HARDWARE_MISSING,
    HARDWARE_UNAVAILABLE,
    SENSOR_INVALID,

    MOTOR_ARMING_DISABLED,
    ACTION_DRIVE_DISABLED,

    SAFETY_LOCKED,
    ESTOP_ACTIVE,
    SAFETY_FAULT,

    AUTONOMY_DISABLED,
    AUTONOMY_NOT_PERMITTED,

    CONTROLLER_REQUIRED,

    CONFIGURATION_DISABLED
};

struct RuntimeCapability {
    bool capable = false;
    bool available = false;
    bool permitted = false;

    CapabilityReason reason = CapabilityReason::NONE;
};

struct RuntimeCapabilities {
    RuntimeCapability drive;
    RuntimeCapability rangeSensor;
    RuntimeCapability imu;
    RuntimeCapability manipulators;
    RuntimeCapability actions;
    RuntimeCapability autonomy;
    RuntimeCapability wifiControl;
    RuntimeCapability localProtocol;
};

RuntimeCapabilities buildRuntimeCapabilities(const RobotAPI& robot);
