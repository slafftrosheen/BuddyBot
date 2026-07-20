#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ControlTypes.h"
#include "NetworkTypes.h"

class WebControlProtocol {
public:
  WebControlProtocol();
  bool parseCommand(const char* jsonStr, RobotCommand& outCmd, bool& isAuthCmd, String& outPin, uint32_t& outMsgId);
  String generateAck(uint32_t msgId, bool ok, const String& message, bool isArmed, int authFlag = -1);
  String generateTelemetry(const TelemetryData& t);

private:
  DriveMode parseDriveMode(const char* modeStr);
  ActionId parseAction(const char* actionStr);
  Mood parseMood(const char* moodStr);
};
