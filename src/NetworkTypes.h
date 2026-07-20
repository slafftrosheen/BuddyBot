#pragma once
#include <Arduino.h>
#include "Types.h"

struct TelemetryData {
  bool isConnected = false;
  bool isArmed = false;
  Mood mood = Mood::IDLE;
  DriveMode driveMode = DriveMode::STOPPED;
  ActionId actionId = ActionId::NONE;
  uint16_t rangeMm = 0;
  bool hasController = false; 
};
