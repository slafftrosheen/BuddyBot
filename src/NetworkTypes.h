#pragma once
#include <Arduino.h>
#include "Types.h"

struct RobotTelemetry {
  uint32_t revision = 0;
  uint32_t uptimeMs = 0;
  bool wifiApRunning = false;
  uint8_t wifiClientCount = 0;
  bool controllerPresent = false;
  bool pairingAvailable = false;
  bool motorsArmed = false;
  bool motorAllowedByFirmware = false;
  DriveMode driveMode = DriveMode::STOPPED;
  const char* driveModeName = nullptr;
  ActionId action = ActionId::NONE;
  const char* actionName = nullptr;
  Mood mood = Mood::IDLE;
  const char* moodName = nullptr;
  uint16_t rangeMm = 0;
  bool rangeValid = false;
  bool obstacleDetected = false; // Legacy, optional to keep
  const char* rangeSensorHealth = "";
  const char* obstacleSafetyState = "";
  bool forwardMotionBlocked = false;
  
  bool autonomyEnabled = false;

  const char* buildName = "";
  const char* personaName = "";
  const char* firmwareVersion = "";
  const char* firmwareChannel = "";
  uint32_t lastSafetyStopMs = 0;
  const char* lastSafetyStopReason = "";
  const char* autonomyMode = "";
  const char* autonomyState = "";
  
  bool hasDrive = false;
  bool fourWheelDrive = false;
  bool hasManipulators = false;
  bool actionRunning = false;
};


