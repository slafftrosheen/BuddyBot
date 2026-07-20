#pragma once
#include <Arduino.h>
#include "Types.h"

// ---------- Firmware ----------
constexpr char FIRMWARE_NAME[] = "BuddyBot OS";
constexpr char FIRMWARE_VERSION[] = "0.2.0";

// ---------- Display ----------
constexpr uint8_t DISPLAY_ROTATION = 0;
constexpr uint16_t SCREEN_W = 135;
constexpr uint16_t SCREEN_H = 240;
constexpr uint16_t FRAME_INTERVAL_MS = 50;

// ---------- I2C bus ----------
constexpr int I2C_SDA_PIN = 9;
constexpr int I2C_SCL_PIN = 10;
constexpr uint32_t I2C_FREQUENCY = 400000;

// ---------- Unit 8Servos ----------
constexpr uint8_t SERVOS8_ADDR = 0x25;

// ---------- Sonic ----------
constexpr uint8_t SONIC_I2C_ADDR = 0x57;

// ---------- Roller ----------
constexpr uint8_t ROLLER1_ADDR = 0x64;
constexpr uint8_t ROLLER2_ADDR = 0x65;
constexpr int16_t ROLLER_MAX_RPM = 100;
constexpr uint16_t ROLLER_SPEED_CURRENT_LIMIT_MA = 500;
constexpr bool ROLLER_LEFT_INVERTED = false;
constexpr bool ROLLER_RIGHT_INVERTED = true;

// ---------- Safety ----------
constexpr bool ALLOW_MOTOR_ARMING = false;
constexpr uint16_t SAFE_DRIVE_TIME_MS = 1200;
constexpr uint16_t ACTION_TIMEOUT_MS[] = {
  0,     // NONE
  3000,  // WAVE
  1500,  // LOOK_LEFT
  1500,  // LOOK_RIGHT
  4000,  // CELEBRATE
  5000,  // DANCE
  2000,  // GREET
  2000   // SLEEP
};
constexpr uint16_t ACTION_STEP_MS = 220;
constexpr uint16_t SERIAL_COMMAND_MAX_LENGTH = 96;

// ---------- Servo pulse bounds ----------
constexpr uint16_t SERVO_MIN_US = 1000;
constexpr uint16_t SERVO_MAX_US = 2000;

// ---------- Servo channels ----------
constexpr uint8_t LEFT_WHEEL_CHANNEL = 0;
constexpr uint8_t RIGHT_WHEEL_CHANNEL = 1;
constexpr uint8_t HEAD_CHANNEL = 2;
constexpr uint8_t LEFT_ARM_CHANNEL = 3;
constexpr uint8_t RIGHT_ARM_CHANNEL = 4;
constexpr uint8_t ACCESSORY_CHANNEL_1 = 5;
constexpr uint8_t ACCESSORY_CHANNEL_2 = 6;
constexpr uint8_t ACCESSORY_CHANNEL_3 = 7;

// ---------- Continuous servo calibration ----------
constexpr uint16_t LEFT_STOP_US = 1500;
constexpr uint16_t LEFT_FORWARD_US = 1600;
constexpr uint16_t LEFT_REVERSE_US = 1400;
constexpr uint16_t RIGHT_STOP_US = 1500;
constexpr uint16_t RIGHT_FORWARD_US = 1400;
constexpr uint16_t RIGHT_REVERSE_US = 1600;

// ---------- Joint angles ----------
constexpr uint8_t HEAD_CENTER_DEG = 90;
constexpr uint8_t HEAD_LEFT_DEG = 120;
constexpr uint8_t HEAD_RIGHT_DEG = 60;

constexpr uint8_t ARM_LEFT_REST_DEG = 90;
constexpr uint8_t ARM_RIGHT_REST_DEG = 90;
constexpr uint8_t ARM_LEFT_UP_DEG = 45;
constexpr uint8_t ARM_RIGHT_UP_DEG = 135;
constexpr uint8_t ARM_LEFT_WAVE_IN_DEG = 65;
constexpr uint8_t ARM_LEFT_WAVE_OUT_DEG = 125;

// ---------- Build enums ----------
enum class DriveControllerType : uint8_t {
  NONE,
  SERVO8_CONTINUOUS,
  ROLLER_UNIT
};

enum class JointControllerType : uint8_t {
  NONE,
  SERVO8_POSITION,
  ROLLER_UNIT
};

enum class RangeSensorType : uint8_t {
  NONE,
  SONIC_I2C
};

enum class AccessoryControllerType : uint8_t {
  NONE,
  SERVO8_POSITION,
  ROLLER_UNIT
};

enum class BuildProfileId : uint8_t {
  STICKY_SERVO_ROVER,
  SERVO_DRIVE_SERVO_ARMS,
  ROLLER_DRIVE_SERVO_ARMS,
  DUAL_ROLLER_DRIVE_ONLY,
  ACCESSORY_DEMO_RIG,
  CUSTOM
};

struct RobotBuildConfig {
  DriveControllerType driveType;
  JointControllerType headType;
  JointControllerType leftArmType;
  JointControllerType rightArmType;
  AccessoryControllerType accessory1Type;
  AccessoryControllerType accessory2Type;
  AccessoryControllerType accessory3Type;
  RangeSensorType rangeSensorType;
  bool useRoller1ForDrive;
  bool useRoller2ForDrive;
  bool enableObstacleStop;
};

// ---------- Active build ----------
static constexpr BuildProfileId ACTIVE_BUILD_PROFILE = BuildProfileId::STICKY_SERVO_ROVER;
static constexpr RobotBuildConfig BUILD = {
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

// ---------- Obstacle / Autonomy ----------
constexpr uint16_t OBSTACLE_STOP_MM = 120;
constexpr uint16_t OBSTACLE_CLEAR_MM = 180;
constexpr uint16_t AUTONOMY_REVERSE_MS = 350;
constexpr uint16_t AUTONOMY_TURN_MS = 500;
constexpr uint16_t AUTONOMY_CLEAR_TIMEOUT_MS = 5000;
constexpr uint16_t AUTONOMY_COMMAND_REFRESH_MS = 150;

// ---------- Persona defaults ----------
constexpr char DEFAULT_PERSONA[] = "PIXEL";

// ---------- Accessories ----------
constexpr bool ENABLE_ANTENNA = true;
constexpr bool ENABLE_STAR_BADGE = true;
constexpr bool ENABLE_VISOR = false;
constexpr bool ENABLE_CHEEK_LEDS = true;

constexpr uint8_t ACCESSORY_REST_DEG = 90;
constexpr uint8_t ACCESSORY_ACTIVE_DEG = 180;

// ---------- Wi-Fi Control ----------
constexpr bool ENABLE_WIFI_CONTROL = true;
constexpr bool WIFI_START_SOFT_AP = true;
constexpr bool WIFI_ENABLE_STA = false;

constexpr char WIFI_AP_SSID[] = "BuddyBot-Control";
constexpr char WIFI_AP_PASSWORD[] = "change-this-before-use";
constexpr uint16_t WIFI_HTTP_PORT = 80;
constexpr char WIFI_WS_PATH[] = "/ws";

constexpr uint16_t WIFI_DRIVE_KEEPALIVE_MS = 150;
constexpr uint16_t WIFI_DRIVE_COMMAND_DURATION_MS = 250;
constexpr uint8_t WIFI_MAX_CLIENTS = 2;
constexpr char WIFI_UI_PIN[] = "1234";
