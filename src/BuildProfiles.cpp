#include "BuildProfiles.h"
#include <Arduino.h>

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
