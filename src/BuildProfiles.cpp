#include "BuildProfiles.h"
#include "ServoConfig.h"
#include <Arduino.h>

const ServoChannelConfig DEFAULT_4WD_SERVO_CONFIG[8] = {
  {0, ServoRole::DRIVE_FRONT_LEFT,  true,  true,  {1500, 1580, 1420, SERVO_MIN_US, SERVO_MAX_US, 0, ServoDirection::NORMAL},   {}},
  {1, ServoRole::DRIVE_REAR_LEFT,   true,  true,  {1500, 1580, 1420, SERVO_MIN_US, SERVO_MAX_US, 0, ServoDirection::NORMAL},   {}},
  {2, ServoRole::DRIVE_FRONT_RIGHT, true,  true,  {1500, 1420, 1580, SERVO_MIN_US, SERVO_MAX_US, 0, ServoDirection::INVERTED}, {}},
  {3, ServoRole::DRIVE_REAR_RIGHT,  true,  true,  {1500, 1420, 1580, SERVO_MIN_US, SERVO_MAX_US, 0, ServoDirection::INVERTED}, {}},
  {4, ServoRole::HEAD,              true,  false, {}, {0, 180, HEAD_CENTER_DEG, false, SERVO_MIN_US, SERVO_MAX_US}},
  {5, ServoRole::LEFT_ARM,          true,  false, {}, {0, 180, ARM_LEFT_REST_DEG, false, SERVO_MIN_US, SERVO_MAX_US}},
  {6, ServoRole::RIGHT_ARM,         true,  false, {}, {0, 180, ARM_RIGHT_REST_DEG, false, SERVO_MIN_US, SERVO_MAX_US}},
  {7, ServoRole::ACCESSORY_1,       true,  false, {}, {0, 180, ACCESSORY_REST_DEG, false, SERVO_MIN_US, SERVO_MAX_US}}
};

const ServoChannelConfig LEGACY_2WD_SERVO_CONFIG[8] = {
  {0, ServoRole::DRIVE_FRONT_LEFT,  true,  true,  {1500, 1600, 1400, SERVO_MIN_US, SERVO_MAX_US, 0, ServoDirection::NORMAL},   {}},
  {1, ServoRole::DRIVE_FRONT_RIGHT, true,  true,  {1500, 1400, 1600, SERVO_MIN_US, SERVO_MAX_US, 0, ServoDirection::INVERTED}, {}},
  {2, ServoRole::HEAD,              true,  false, {}, {0, 180, HEAD_CENTER_DEG, false, SERVO_MIN_US, SERVO_MAX_US}},
  {3, ServoRole::LEFT_ARM,          true,  false, {}, {0, 180, ARM_LEFT_REST_DEG, false, SERVO_MIN_US, SERVO_MAX_US}},
  {4, ServoRole::RIGHT_ARM,         true,  false, {}, {0, 180, ARM_RIGHT_REST_DEG, false, SERVO_MIN_US, SERVO_MAX_US}},
  {5, ServoRole::ACCESSORY_1,       true,  false, {}, {0, 180, ACCESSORY_REST_DEG, false, SERVO_MIN_US, SERVO_MAX_US}},
  {6, ServoRole::UNUSED,            false, false, {}, {}},
  {7, ServoRole::UNUSED,            false, false, {}, {}}
};

const RobotBuildConfig CUSTOM_BUILD = {
  DriveControllerType::SERVO8_CONTINUOUS,
  JointControllerType::SERVO8_POSITION,
  JointControllerType::SERVO8_POSITION,
  JointControllerType::SERVO8_POSITION,
  AccessoryControllerType::SERVO8_POSITION,
  AccessoryControllerType::SERVO8_POSITION,
  AccessoryControllerType::SERVO8_POSITION,
  RangeSensorType::SONIC_I2C,
  false,
  false,
  true
};

const NamedBuildProfile BUILD_PROFILES[] = {
  {
    BuildProfileId::SERVO8_FOUR_WHEEL_MANIPULATOR,
    "Servo8 4WD + Manipulator",
    {
      DriveControllerType::SERVO8_CONTINUOUS,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      AccessoryControllerType::NONE,
      AccessoryControllerType::NONE,
      RangeSensorType::SONIC_I2C,
      false,
      false,
      true
    }
  },
  {
    BuildProfileId::SERVO8_TWO_WHEEL_MANIPULATOR,
    "Servo8 2WD + Manipulator",
    {
      DriveControllerType::SERVO8_CONTINUOUS,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      AccessoryControllerType::NONE,
      AccessoryControllerType::NONE,
      RangeSensorType::SONIC_I2C,
      false,
      false,
      true
    }
  },
  {
    BuildProfileId::STICKY_SERVO_ROVER,
    "Sticky Servo Rover",
    {
      DriveControllerType::SERVO8_CONTINUOUS,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      RangeSensorType::SONIC_I2C,
      false,
      false,
      true
    }
  },
  {
    BuildProfileId::SERVO_DRIVE_SERVO_ARMS,
    "Servo Drive + Servo Arms",
    {
      DriveControllerType::SERVO8_CONTINUOUS,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      AccessoryControllerType::NONE,
      AccessoryControllerType::NONE,
      RangeSensorType::SONIC_I2C,
      false,
      false,
      true
    }
  },
  {
    BuildProfileId::ROLLER_DRIVE_SERVO_ARMS,
    "Roller Drive + Servo Arms",
    {
      DriveControllerType::ROLLER_UNIT,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      AccessoryControllerType::NONE,
      AccessoryControllerType::NONE,
      RangeSensorType::SONIC_I2C,
      true,
      true,
      true
    }
  },
  {
    BuildProfileId::DUAL_ROLLER_DRIVE_ONLY,
    "Dual Roller Drive",
    {
      DriveControllerType::ROLLER_UNIT,
      JointControllerType::NONE,
      JointControllerType::NONE,
      JointControllerType::NONE,
      AccessoryControllerType::NONE,
      AccessoryControllerType::NONE,
      AccessoryControllerType::NONE,
      RangeSensorType::SONIC_I2C,
      true,
      true,
      true
    }
  },
  {
    BuildProfileId::ACCESSORY_DEMO_RIG,
    "Accessory Demo Rig",
    {
      DriveControllerType::NONE,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      JointControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      AccessoryControllerType::SERVO8_POSITION,
      RangeSensorType::SONIC_I2C,
      false,
      false,
      false
    }
  },
  {
    BuildProfileId::CUSTOM,
    "Custom",
    CUSTOM_BUILD
  }
};

const size_t BUILD_PROFILE_COUNT = sizeof(BUILD_PROFILES) / sizeof(BUILD_PROFILES[0]);

static const char* driveName(DriveControllerType t) {
  switch (t) {
    case DriveControllerType::SERVO8_CONTINUOUS: return "SERVO8_CONTINUOUS";
    case DriveControllerType::ROLLER_UNIT: return "ROLLER_UNIT";
    default: return "NONE";
  }
}

static const char* jointName(JointControllerType t) {
  switch (t) {
    case JointControllerType::SERVO8_POSITION: return "SERVO8_POSITION";
    case JointControllerType::ROLLER_UNIT: return "ROLLER_UNIT";
    default: return "NONE";
  }
}

static const char* accName(AccessoryControllerType t) {
  switch (t) {
    case AccessoryControllerType::SERVO8_POSITION: return "SERVO8_POSITION";
    case AccessoryControllerType::ROLLER_UNIT: return "ROLLER_UNIT";
    default: return "NONE";
  }
}

static const char* rangeName(RangeSensorType t) {
  switch (t) {
    case RangeSensorType::SONIC_I2C: return "SONIC_I2C";
    default: return "NONE";
  }
}

const NamedBuildProfile* findBuildProfile(BuildProfileId id) {
  for (size_t i = 0; i < BUILD_PROFILE_COUNT; ++i) {
    if (BUILD_PROFILES[i].id == id) return &BUILD_PROFILES[i];
  }
  return nullptr;
}

RobotBuildConfig getActiveBuildConfig() {
  const NamedBuildProfile* p = findBuildProfile(ACTIVE_BUILD_PROFILE);
  if (!p) return CUSTOM_BUILD;
  if (p->id == BuildProfileId::CUSTOM) return CUSTOM_BUILD;
  return p->config;
}

const char* getActiveBuildName() {
  const NamedBuildProfile* p = findBuildProfile(ACTIVE_BUILD_PROFILE);
  return p ? p->name : "Custom";
}

const ServoChannelConfig* getActiveServoConfig() {
  if (ACTIVE_BUILD_PROFILE == BuildProfileId::SERVO8_TWO_WHEEL_MANIPULATOR) {
    return LEGACY_2WD_SERVO_CONFIG;
  }
  // Default to 4WD mapping for all other profiles assuming they use Servo8
  return DEFAULT_4WD_SERVO_CONFIG;
}

void printBuildProfile(const RobotBuildConfig& config) {
  Serial.printf("drive=%s\n", driveName(config.driveType));
  Serial.printf("head=%s\n", jointName(config.headType));
  Serial.printf("leftArm=%s\n", jointName(config.leftArmType));
  Serial.printf("rightArm=%s\n", jointName(config.rightArmType));
  Serial.printf("accessory1=%s\n", accName(config.accessory1Type));
  Serial.printf("accessory2=%s\n", accName(config.accessory2Type));
  Serial.printf("accessory3=%s\n", accName(config.accessory3Type));
  Serial.printf("range=%s\n", rangeName(config.rangeSensorType));
  Serial.printf("roller1Drive=%s\n", config.useRoller1ForDrive ? "true" : "false");
  Serial.printf("roller2Drive=%s\n", config.useRoller2ForDrive ? "true" : "false");
  Serial.printf("obstacleStop=%s\n", config.enableObstacleStop ? "true" : "false");
}

void printAllBuildProfiles() {
  for (size_t i = 0; i < BUILD_PROFILE_COUNT; ++i) {
    Serial.printf("%u: %s\n", unsigned(BUILD_PROFILES[i].id), BUILD_PROFILES[i].name);
  }
}

bool validateBuildConfig(const RobotBuildConfig& config, const ServoChannelConfig* servos) {
  bool valid = true;
  
  // Check for duplicate servo channels if using SERVO8
  if (config.driveType == DriveControllerType::SERVO8_CONTINUOUS ||
      config.headType == JointControllerType::SERVO8_POSITION) {
    if (!servos) {
      Serial.println("ERR: Servo config missing");
      return false;
    }
    
    bool seen[8] = {false};
    for (int i = 0; i < 8; i++) {
      if (servos[i].enabled) {
        if (servos[i].channel >= 8) {
          Serial.printf("ERR: Invalid channel %u for role %u\n", servos[i].channel, (unsigned)servos[i].role);
          valid = false;
        } else if (seen[servos[i].channel]) {
          Serial.printf("ERR: Duplicate channel %u\n", servos[i].channel);
          valid = false;
        } else {
          seen[servos[i].channel] = true;
        }
      }
    }
  }
  
  return valid;
}
