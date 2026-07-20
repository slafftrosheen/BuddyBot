#pragma once
#include <Arduino.h>
#include "Types.h"

struct RobotTelemetry {
  uint32_t revision = 0;
  uint32_t uptimeMs = 0;
  bool wifiApRunning = false;
  uint8_t wifiClientCount = 0;
  bool controllerPresent = false;
  bool motorsArmed = false;
  bool motorAllowedByFirmware = false;
  DriveMode driveMode = DriveMode::STOPPED;
  ActionId action = ActionId::NONE;
  Mood mood = Mood::IDLE;
  uint16_t rangeMm = 0;
  bool rangeValid = false;
  bool obstacleDetected = false;
  bool autonomyEnabled = false;
  const char* buildName = "";
  const char* personaName = "";
  uint32_t lastSafetyStopMs = 0;
  const char* lastSafetyStopReason = "";
};

struct EventLogEntry {
  uint32_t timestampMs = 0;
  const char* severity = "INFO";
  const char* code = "";
};
