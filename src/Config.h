#pragma once
#include <Arduino.h>
#include "Types.h"

#if !__has_include("arduino_secrets.h")
#error "Create src/arduino_secrets.h from src/arduino_secrets.example.h before building."
#endif
#include "arduino_secrets.h"

// ---------- Firmware ----------
constexpr char FIRMWARE_NAME[] = "BuddyBot OS";
constexpr char FIRMWARE_VERSION[] = "0.3.0-beta.1";
constexpr char FIRMWARE_CHANNEL[] = "hardware-safety-beta";
constexpr uint8_t CONTROL_PROTOCOL_VERSION = 1;
constexpr uint16_t CONFIG_SCHEMA_VERSION = 1;
constexpr uint16_t HARDWARE_MANIFEST_VERSION = 1;
constexpr uint16_t SAFETY_POLICY_VERSION = 1;

// ---------- Display ----------
constexpr uint8_t DISPLAY_ROTATION = 0;
constexpr uint16_t SCREEN_W = 135;
constexpr uint16_t SCREEN_H = 240;
constexpr uint16_t FRAME_INTERVAL_MS = 50;

// ---------- Expressions & Persona ----------
constexpr uint16_t EXPRESSION_DEFAULT_MS = 900;
constexpr uint16_t EXPRESSION_SHORT_MS = 500;
constexpr uint16_t EXPRESSION_LONG_MS = 1800;
constexpr uint16_t PERSONA_IDLE_MIN_MS = 1800;
constexpr uint16_t PERSONA_IDLE_MAX_MS = 5000;
constexpr uint16_t BATTERY_UI_UPDATE_DELTA_PERCENT = 2;
constexpr uint16_t RANGE_UI_UPDATE_DELTA_MM = 20;
constexpr bool ENABLE_PERSONA_PARTICLES = true;
constexpr uint8_t MAX_PERSONA_PARTICLES = 8;

// ---------- Built-in BMI270 IMU ----------
constexpr uint16_t IMU_SAMPLE_INTERVAL_MS = 100;
constexpr uint16_t IMU_MAX_SAMPLE_AGE_MS = 500;
constexpr float IMU_MIN_ACCEL_G = 0.25f;
constexpr float IMU_MAX_ACCEL_G = 3.0f;
constexpr float IMU_MAX_TILT_DEG = 35.0f;
constexpr float IMU_MAX_GYRO_DPS = 720.0f;

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

// ---------- SAFE DEFAULTS ----------
constexpr bool ALLOW_MOTOR_ARMING = false;
constexpr bool ALLOW_ACTION_DRIVE_MOVEMENT = false;
constexpr bool RETURN_MANIPULATORS_ON_SAFETY_STOP = false;
constexpr uint16_t SAFE_DRIVE_TIME_MS = 1200;

constexpr bool ENABLE_AUTONOMY_AT_BOOT = false;
constexpr bool ENABLE_CAUTIOUS_ROAM = false;

constexpr bool ENABLE_OBSTACLE_SAFETY = true;
constexpr bool REQUIRE_VALID_RANGE_FOR_FORWARD_DRIVE = true;
constexpr uint16_t SAFETY_MANUAL_OVERRIDE_MS = 600;
constexpr uint16_t PHYSICAL_ESTOP_HOLD_MS = 1200;
constexpr uint16_t PHYSICAL_ESTOP_RESET_HOLD_MS = 2500;
constexpr uint16_t SYSTEM_TASK_WATCHDOG_TIMEOUT_MS = 3000;

static_assert(!ALLOW_MOTOR_ARMING || ENABLE_OBSTACLE_SAFETY, "Motor arming should not be enabled without obstacle safety");
static_assert(!ENABLE_CAUTIOUS_ROAM || ALLOW_MOTOR_ARMING, "Cautious roam requires motor arming");
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

// ---------- Servo timing ----------
constexpr uint16_t SERVO_DRIVE_STARTUP_SETTLE_MS = 250;
constexpr uint16_t SERVO_MANIPULATOR_STEP_MIN_MS = 80;
constexpr uint16_t SERVO_MANIPULATOR_STEP_MAX_MS = 3000;

// ---------- Servo pulse bounds ----------
constexpr uint16_t SERVO_MIN_US = 1000;
constexpr uint16_t SERVO_MAX_US = 2000;

// ---------- Servo channels ----------
constexpr uint8_t SERVO8_CHANNEL_COUNT = 8;
constexpr uint8_t DRIVE_SERVO_CHANNEL_COUNT = 4;
constexpr uint8_t MANIPULATOR_SERVO_CHANNEL_COUNT = 4;

constexpr uint8_t DRIVE_FRONT_LEFT_CHANNEL  = 0;
constexpr uint8_t DRIVE_REAR_LEFT_CHANNEL   = 1;
constexpr uint8_t DRIVE_FRONT_RIGHT_CHANNEL = 2;
constexpr uint8_t DRIVE_REAR_RIGHT_CHANNEL  = 3;

constexpr uint8_t HEAD_CHANNEL              = 4;
constexpr uint8_t LEFT_ARM_CHANNEL          = 5;
constexpr uint8_t RIGHT_ARM_CHANNEL         = 6;
constexpr uint8_t ACCESSORY_CHANNEL_1       = 7;

static_assert(DRIVE_FRONT_LEFT_CHANNEL < SERVO8_CHANNEL_COUNT, "Channel out of range");
static_assert(DRIVE_REAR_LEFT_CHANNEL < SERVO8_CHANNEL_COUNT, "Channel out of range");
static_assert(DRIVE_FRONT_RIGHT_CHANNEL < SERVO8_CHANNEL_COUNT, "Channel out of range");
static_assert(DRIVE_REAR_RIGHT_CHANNEL < SERVO8_CHANNEL_COUNT, "Channel out of range");
static_assert(HEAD_CHANNEL < SERVO8_CHANNEL_COUNT, "Channel out of range");
static_assert(LEFT_ARM_CHANNEL < SERVO8_CHANNEL_COUNT, "Channel out of range");
static_assert(RIGHT_ARM_CHANNEL < SERVO8_CHANNEL_COUNT, "Channel out of range");
static_assert(ACCESSORY_CHANNEL_1 < SERVO8_CHANNEL_COUNT, "Channel out of range");

// Duplicate checks removed for C++11 compatibility

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
  SERVO8_FOUR_WHEEL_MANIPULATOR,
  SERVO8_TWO_WHEEL_MANIPULATOR,
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
static constexpr BuildProfileId ACTIVE_BUILD_PROFILE = BuildProfileId::SERVO8_FOUR_WHEEL_MANIPULATOR;
static constexpr RobotBuildConfig BUILD = {
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
};

// ---------- Sonic Range Sensor ----------
constexpr uint16_t SONIC_SAMPLE_INTERVAL_MS = 80;
constexpr uint16_t SONIC_MIN_VALID_MM = 20;
constexpr uint16_t SONIC_MAX_VALID_MM = 3500;
constexpr uint16_t SONIC_MAX_STEP_MM = 700;
constexpr uint16_t SONIC_STALE_AFTER_MS = 500;
constexpr uint8_t SONIC_INVALID_SAMPLES_TO_FAULT = 4;

// ---------- Obstacle Safety Config ----------
constexpr uint16_t OBSTACLE_EMERGENCY_STOP_MM = 120;
constexpr uint16_t OBSTACLE_CAUTION_MM = 220;
constexpr uint16_t OBSTACLE_CLEAR_MM = 280;
constexpr uint16_t OBSTACLE_HYSTERESIS_MS = 250;

// ---------- Assisted Autonomy Config ----------
constexpr uint16_t AUTONOMY_STOP_SETTLE_MS = 120;
constexpr uint16_t AUTONOMY_REVERSE_MS = 280;
constexpr uint16_t AUTONOMY_TURN_MS = 420;
constexpr uint16_t AUTONOMY_MAX_RECOVERY_MS = 1500;
constexpr uint16_t AUTONOMY_CLEAR_TIMEOUT_MS = 5000;
constexpr uint16_t AUTONOMY_COMMAND_REFRESH_MS = 150;

// ---------- Persona defaults ----------
constexpr char DEFAULT_PERSONA[] = "PIXEL";

// ---------- Accessories ----------
constexpr bool ENABLE_ANTENNA = true;
constexpr bool ENABLE_STAR_BADGE = true;
constexpr bool ENABLE_VISOR = false;
constexpr bool ENABLE_CHEEK_LEDS = true;

constexpr uint16_t ACCESSORY_REST_DEG = 90;
constexpr uint16_t ACCESSORY_ACTIVE_DEG = 120;

// ---------- Joint Control ----------
constexpr uint16_t SERVO_JOINT_UPDATE_INTERVAL_MS = 20;

// ---------- Wi-Fi Control ----------
constexpr bool ENABLE_WIFI_CONTROL = true;
constexpr bool WIFI_START_SOFT_AP = true;
constexpr bool WIFI_ENABLE_STA = false;

#ifndef SECRET_WIFI_AP_SSID
#error "SECRET_WIFI_AP_SSID must be provisioned in src/arduino_secrets.h."
#endif

#ifndef SECRET_WIFI_AP_PASSWORD
#error "SECRET_WIFI_AP_PASSWORD must be provisioned in src/arduino_secrets.h."
#endif

constexpr char WIFI_AP_SSID[] = SECRET_WIFI_AP_SSID;
constexpr char WIFI_AP_PASSWORD[] = SECRET_WIFI_AP_PASSWORD;
static_assert(sizeof(WIFI_AP_PASSWORD) - 1 >= 12, "WIFI_AP_PASSWORD must be at least 12 characters long");

constexpr uint16_t WIFI_HTTP_PORT = 80;
constexpr char WIFI_WS_PATH[] = "/ws";

constexpr size_t WIFI_MAX_WS_FRAME_BYTES = 256;
constexpr size_t WIFI_JSON_DOCUMENT_BYTES = 512;
constexpr uint8_t WIFI_MAX_CLIENTS = 2;

constexpr uint8_t WIFI_MAX_COMMANDS_PER_SECOND = 20;
constexpr uint8_t WIFI_MAX_PAIR_ATTEMPTS_PER_MINUTE = 5;
constexpr uint32_t WIFI_PAIRING_LOCKOUT_MS = 60000;
constexpr uint32_t WIFI_UNPAIRED_CLIENT_TIMEOUT_MS = 30000;

constexpr uint16_t WIFI_TELEMETRY_INTERVAL_MS = 250;
constexpr uint16_t WIFI_DRIVE_KEEPALIVE_MS = 150;
constexpr uint16_t WIFI_DRIVE_COMMAND_DURATION_MS = 250;
constexpr uint16_t WIFI_DRIVE_WATCHDOG_MS = 350;
constexpr uint32_t WIFI_CONTROLLER_LEASE_MS = 10000;
constexpr uint32_t WIFI_PAIRING_CODE_LIFETIME_MS = 600000;

constexpr uint8_t WIFI_PAIRING_DIGITS = 8;
constexpr uint8_t WIFI_SESSION_TOKEN_BYTES = 16;
constexpr uint8_t WIFI_SESSION_TOKEN_HEX_CHARS = 32;

constexpr size_t WIFI_COMMAND_QUEUE_CAPACITY = 16;
constexpr size_t WIFI_EVENT_LOG_CAPACITY = 16;
