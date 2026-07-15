#pragma once
#include <Arduino.h>
#include "Types.h"

enum class ControlSource : uint8_t {
  SERIAL_CTRL,
  WIFI,
  BLE,
  LOCAL_UI,
  AUTONOMY
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
  AUTONOMY_SET
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
};
