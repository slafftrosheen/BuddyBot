#include "RuntimeCapabilities.h"
#include "RobotAPI.h"
#include "Config.h"

RuntimeCapabilities buildRuntimeCapabilities(const RobotAPI& robot) {
    RuntimeCapabilities caps;

    // 1. DRIVE
    caps.drive.capable = true;
    caps.drive.available = robot.driveAvailable();
    if (!ALLOW_MOTOR_ARMING) {
        caps.drive.permitted = false;
        caps.drive.reason = CapabilityReason::MOTOR_ARMING_DISABLED;
    } else if (robot.safetyState() == SafetyState::ESTOP) {
        caps.drive.permitted = false;
        caps.drive.reason = CapabilityReason::ESTOP_ACTIVE;
    } else if (robot.safetyFault() != SafetyFault::NONE) {
        caps.drive.permitted = false;
        caps.drive.reason = CapabilityReason::SAFETY_FAULT;
    } else {
        caps.drive.permitted = robot.mayEnableDrive();
        if (!caps.drive.permitted) {
            caps.drive.reason = CapabilityReason::SAFETY_LOCKED;
        }
    }

    // 2. RANGE SENSOR
    caps.rangeSensor.capable = true;
    bool rangeMissing = (robot.rangeSensorHealth() == RangeSensorHealth::UNAVAILABLE || robot.rangeSensorHealth() == RangeSensorHealth::UNINITIALIZED);
    caps.rangeSensor.available = !rangeMissing;
    if (rangeMissing) {
        caps.rangeSensor.permitted = false;
        caps.rangeSensor.reason = CapabilityReason::HARDWARE_MISSING;
    } else if (robot.rangeSensorHealth() != RangeSensorHealth::READY && robot.rangeSensorHealth() != RangeSensorHealth::STALE) {
        caps.rangeSensor.permitted = false;
        caps.rangeSensor.reason = CapabilityReason::SENSOR_INVALID;
    } else {
        caps.rangeSensor.permitted = true;
    }

    // 3. IMU
    caps.imu.capable = true;
    caps.imu.available = robot.imuReading().available;
    if (!caps.imu.available) {
        caps.imu.permitted = false;
        caps.imu.reason = CapabilityReason::HARDWARE_MISSING;
    } else if (!robot.imuReading().valid) {
        caps.imu.permitted = false;
        caps.imu.reason = CapabilityReason::SENSOR_INVALID;
    } else {
        caps.imu.permitted = true;
    }

    // 4. MANIPULATORS
    const RobotBuildConfig& cfg = robot.buildConfig();
    bool hasManipulators = (cfg.headType != JointControllerType::NONE || 
                            cfg.leftArmType != JointControllerType::NONE || 
                            cfg.rightArmType != JointControllerType::NONE);
    if (hasManipulators) {
        caps.manipulators.capable = true;
        caps.manipulators.available = robot.actuatorCapabilities().servoBusConnected;
        
        if (!caps.manipulators.available) {
            caps.manipulators.permitted = false;
            caps.manipulators.reason = CapabilityReason::HARDWARE_MISSING;
        } else if (robot.safetyState() == SafetyState::ESTOP) {
            caps.manipulators.permitted = false;
            caps.manipulators.reason = CapabilityReason::ESTOP_ACTIVE;
        } else if (robot.safetyFault() != SafetyFault::NONE) {
            caps.manipulators.permitted = false;
            caps.manipulators.reason = CapabilityReason::SAFETY_FAULT;
        } else {
            caps.manipulators.permitted = robot.mayMoveManipulators();
            if (!caps.manipulators.permitted) {
                caps.manipulators.reason = CapabilityReason::SAFETY_LOCKED;
            }
        }
    } else {
        caps.manipulators.capable = false;
        caps.manipulators.available = false;
        caps.manipulators.permitted = false;
        caps.manipulators.reason = CapabilityReason::NOT_IMPLEMENTED;
    }

    // 5. ACTIONS
    caps.actions.capable = true;
    caps.actions.available = true;
    if (robot.safetyState() == SafetyState::ESTOP) {
        caps.actions.permitted = false;
        caps.actions.reason = CapabilityReason::ESTOP_ACTIVE;
    } else if (robot.safetyFault() != SafetyFault::NONE) {
        caps.actions.permitted = false;
        caps.actions.reason = CapabilityReason::SAFETY_FAULT;
    } else {
        caps.actions.permitted = true;
    }

    // 6. AUTONOMY
    caps.autonomy.capable = true;
    caps.autonomy.available = true;
    if (!robot.autonomyEnabled()) {
        caps.autonomy.permitted = false;
        caps.autonomy.reason = CapabilityReason::AUTONOMY_DISABLED;
    } else if (!robot.autonomyMotionAllowed()) {
        caps.autonomy.permitted = false;
        caps.autonomy.reason = CapabilityReason::AUTONOMY_NOT_PERMITTED;
    } else if (robot.safetyState() == SafetyState::ESTOP) {
        caps.autonomy.permitted = false;
        caps.autonomy.reason = CapabilityReason::ESTOP_ACTIVE;
    } else if (robot.safetyFault() != SafetyFault::NONE) {
        caps.autonomy.permitted = false;
        caps.autonomy.reason = CapabilityReason::SAFETY_FAULT;
    } else {
        caps.autonomy.permitted = true;
    }

    // 7. WIFI CONTROL
    caps.wifiControl.capable = true;
    caps.wifiControl.available = ENABLE_WIFI_CONTROL;
    if (!caps.wifiControl.available) {
        caps.wifiControl.permitted = false;
        caps.wifiControl.reason = CapabilityReason::CONFIGURATION_DISABLED;
    } else {
        caps.wifiControl.permitted = true;
    }

    // 8. LOCAL PROTOCOL
    caps.localProtocol.capable = true;
    caps.localProtocol.available = true;
    caps.localProtocol.permitted = true;

    return caps;
}
