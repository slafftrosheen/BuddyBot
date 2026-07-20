#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "ControlTypes.h"
#include "NetworkTypes.h"

class WebControlProtocol {
public:
  WebControlProtocol();
  bool parseCommand(
    const char* jsonStr, size_t len,
    RobotCommand& outCmd,
    String& outType,
    String& outToken,
    String& outCode,
    uint32_t& outMsgId,
    String& outErrorCode
  );

  String generateAck(uint32_t msgId, bool ok, const String& codeOrMessage, uint32_t revision);
  String generateError(uint32_t msgId, const String& code);
  String generateTelemetry(const RobotTelemetry& t);
  String generateEventLog(const EventLogEntry* entries, size_t count);

private:
  DriveMode parseDriveMode(const char* modeStr);
  ActionId parseAction(const char* actionStr);
  Mood parseMood(const char* moodStr);
};
