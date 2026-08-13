#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "ControlTypes.h"
#include "NetworkTypes.h"
#include "EventLog.h"

enum class WebMessageType : uint8_t {
  INVALID,
  HELLO,
  PING,
  STATUS,
  PAIR,
  MOVE,
  STOP,
  ARM,
  DISARM,
  ACTION,
  MOOD,
  PERSONA_NEXT,
  ACCESSORY
};

enum class WebProtocolError : uint8_t {
  NONE,
  FRAME_TOO_LARGE,
  BAD_JSON,
  UNSUPPORTED_VERSION,
  UNKNOWN_TYPE,
  INVALID_ARGUMENT,
  NOT_CONTROLLER,
  BAD_TOKEN,
  CONTROLLER_BUSY,
  PAIRING_UNAVAILABLE,
  PAIRING_FAILED,
  RATE_LIMITED,
  REPLAYED_COMMAND,
  QUEUE_FULL,
  SUPERSEDED,
  MOTORS_LOCKED,
  INTERNAL_ERROR
};

struct WebParsedMessage {
  WebMessageType type = WebMessageType::INVALID;
  RobotCommand command {};
  uint32_t requestId = 0;

  bool hasRequestId = false;
  bool hasToken = false;
  char token[WIFI_SESSION_TOKEN_HEX_CHARS + 1] = {};

  char pairingDigits[WIFI_PAIRING_DIGITS + 1] = {};
};

class WebControlProtocol {
public:
  WebControlProtocol();
  
  bool parseCommand(
    const uint8_t* data,
    size_t len,
    WebParsedMessage& out,
    WebProtocolError& error
  );

  const char* webProtocolErrorName(WebProtocolError err) const;
  const char* webMessageTypeName(WebMessageType type) const;

  String generateAck(uint32_t msgId, bool ok, const String& codeOrMessage, uint32_t revision);
  String generateError(uint32_t msgId, const String& code);
  String generateTelemetry(const RobotTelemetry& t);
  String generateEventLog(const EventLogEntry* entries, size_t count);

private:
  DriveMode parseDriveMode(const char* modeStr);
  ActionId parseAction(const char* actionStr);
  Mood parseMood(const char* moodStr);
};
