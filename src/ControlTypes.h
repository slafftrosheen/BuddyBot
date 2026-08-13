#pragma once
#include <Arduino.h>
#include "Types.h"

enum class ControlSource : uint8_t {
  SERIAL_CTRL,
  WIFI,
  BLE,
  LOCAL_UI,
  AUTONOMY,
  EXPRESSION
};

enum class CommandKind : uint8_t {
  NONE,
  ARM,
  DISARM,
  STOP,
  SET_MOOD,
  NEXT_PERSONA,
  MOVE,
  ACTION,
  ACCESSORY,
  RANGE_QUERY,
  PROFILE_LIST,
  PROFILE_SHOW,
  STATUS_QUERY,
  CTRL_SENSOR_STATUS,
  CTRL_SAFETY_STATUS,
  CTRL_AUTONOMY_STATUS,
  AUTONOMY_SET,
  PLAY_EXPRESSION,
  SET_ATTENTION,
  CTRL_WIFI_STATUS,
  CTRL_WIFI_ON,
  CTRL_WIFI_OFF,
  CTRL_WIFI_PAIR,
  SERVO_TEST,
  JOINT_MOVE,
  JOINT_REST,
  VERSION,
  DIAG_BOOT,
  PRINT_EVENTS,
  CTRL_IMU_STATUS
};

struct RobotCommand {
  CommandKind kind = CommandKind::NONE;
  ControlSource source = ControlSource::SERIAL_CTRL;

  Mood mood = Mood::IDLE;
  DriveMode driveMode = DriveMode::STOPPED;
  ActionId action = ActionId::NONE;

  uint8_t index = 0;
  bool flag = false;
  uint16_t durationMs = 0;
  ExpressionId expression = ExpressionId::NONE;
  AttentionTarget attention = AttentionTarget::CENTER;
  
  // For diagnostics / joint
  int16_t arg1 = 0;
  int16_t arg2 = 0;
  
  uint32_t correlationId = 0;
};
