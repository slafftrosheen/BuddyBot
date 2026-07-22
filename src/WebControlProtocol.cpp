#include "WebControlProtocol.h"
#include "Config.h"

WebControlProtocol::WebControlProtocol() {}

const char* WebControlProtocol::webProtocolErrorName(WebProtocolError err) const {
  switch (err) {
    case WebProtocolError::NONE: return "none";
    case WebProtocolError::FRAME_TOO_LARGE: return "frame_too_large";
    case WebProtocolError::BAD_JSON: return "bad_json";
    case WebProtocolError::UNSUPPORTED_VERSION: return "unsupported_version";
    case WebProtocolError::UNKNOWN_TYPE: return "unknown_type";
    case WebProtocolError::INVALID_ARGUMENT: return "invalid_argument";
    case WebProtocolError::NOT_CONTROLLER: return "not_controller";
    case WebProtocolError::BAD_TOKEN: return "bad_token";
    case WebProtocolError::CONTROLLER_BUSY: return "controller_busy";
    case WebProtocolError::PAIRING_UNAVAILABLE: return "pairing_unavailable";
    case WebProtocolError::PAIRING_FAILED: return "pairing_failed";
    case WebProtocolError::RATE_LIMITED: return "rate_limited";
    case WebProtocolError::QUEUE_FULL: return "queue_full";
    case WebProtocolError::SUPERSEDED: return "superseded";
    case WebProtocolError::MOTORS_LOCKED: return "motors_locked";
    case WebProtocolError::INTERNAL_ERROR: return "internal_error";
    default: return "internal_error";
  }
}

const char* WebControlProtocol::webMessageTypeName(WebMessageType type) const {
  switch (type) {
    case WebMessageType::INVALID: return "invalid";
    case WebMessageType::HELLO: return "hello";
    case WebMessageType::PING: return "ping";
    case WebMessageType::STATUS: return "status";
    case WebMessageType::PAIR: return "pair";
    case WebMessageType::MOVE: return "move";
    case WebMessageType::STOP: return "stop";
    case WebMessageType::ARM: return "arm";
    case WebMessageType::DISARM: return "disarm";
    case WebMessageType::ACTION: return "action";
    case WebMessageType::MOOD: return "mood";
    case WebMessageType::PERSONA_NEXT: return "persona_next";
    case WebMessageType::ACCESSORY: return "accessory";
    default: return "unknown";
  }
}

bool WebControlProtocol::parseCommand(
  const uint8_t* data,
  size_t len,
  WebParsedMessage& out,
  WebProtocolError& error
) {
  out = WebParsedMessage{};
  out.command.kind = CommandKind::NONE;
  out.command.source = ControlSource::WIFI;
  error = WebProtocolError::NONE;

  if (len == 0 || len > WIFI_MAX_WS_FRAME_BYTES) {
    error = WebProtocolError::FRAME_TOO_LARGE;
    return false;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, data, len, DeserializationOption::NestingLimit(4));
  
  if (err) {
    error = WebProtocolError::BAD_JSON;
    return false;
  }

  if (!doc["v"].is<int>() || doc["v"].as<int>() != 1) {
    error = WebProtocolError::UNSUPPORTED_VERSION;
    return false;
  }

  if (doc["id"].is<uint32_t>()) {
    out.requestId = doc["id"].as<uint32_t>();
    out.hasRequestId = true;
  }
  
  const char* typeStr = doc["type"];
  if (!typeStr) {
    error = WebProtocolError::UNKNOWN_TYPE;
    return false;
  }

  if (doc["token"].is<const char*>()) {
    const char* t = doc["token"].as<const char*>();
    size_t tLen = strlen(t);
    if (tLen == WIFI_SESSION_TOKEN_HEX_CHARS) {
      bool validHex = true;
      for (size_t i = 0; i < tLen; i++) {
        if (!isxdigit(t[i])) validHex = false;
      }
      if (validHex) {
        strlcpy(out.token, t, sizeof(out.token));
        out.hasToken = true;
      }
    }
  }

  if (strcmp(typeStr, "hello") == 0) out.type = WebMessageType::HELLO;
  else if (strcmp(typeStr, "ping") == 0) out.type = WebMessageType::PING;
  else if (strcmp(typeStr, "status") == 0) out.type = WebMessageType::STATUS;
  else if (strcmp(typeStr, "pair") == 0) out.type = WebMessageType::PAIR;
  else if (strcmp(typeStr, "move") == 0) out.type = WebMessageType::MOVE;
  else if (strcmp(typeStr, "stop") == 0) out.type = WebMessageType::STOP;
  else if (strcmp(typeStr, "arm") == 0) out.type = WebMessageType::ARM;
  else if (strcmp(typeStr, "disarm") == 0) out.type = WebMessageType::DISARM;
  else if (strcmp(typeStr, "action") == 0) out.type = WebMessageType::ACTION;
  else if (strcmp(typeStr, "mood") == 0) out.type = WebMessageType::MOOD;
  else if (strcmp(typeStr, "persona_next") == 0) out.type = WebMessageType::PERSONA_NEXT;
  else if (strcmp(typeStr, "accessory") == 0) out.type = WebMessageType::ACCESSORY;
  else {
    error = WebProtocolError::UNKNOWN_TYPE;
    return false;
  }

  switch (out.type) {
    case WebMessageType::HELLO:
    case WebMessageType::PING:
    case WebMessageType::STATUS:
      return true;

    case WebMessageType::PAIR: {
      if (doc["code"].is<const char*>()) {
        const char* c = doc["code"].as<const char*>();
        size_t dIdx = 0;
        for (size_t i = 0; c[i] != '\0' && dIdx < WIFI_PAIRING_DIGITS; i++) {
          if (isdigit(c[i])) {
            out.pairingDigits[dIdx++] = c[i];
          } else if (c[i] != ' ') {
            error = WebProtocolError::INVALID_ARGUMENT;
            return false;
          }
        }
        if (dIdx == WIFI_PAIRING_DIGITS) {
          out.pairingDigits[dIdx] = '\0';
          return true;
        }
      }
      error = WebProtocolError::INVALID_ARGUMENT;
      return false;
    }

    case WebMessageType::STOP:
      out.command.kind = CommandKind::STOP;
      return true;

    case WebMessageType::ARM:
      if (!out.hasRequestId || !out.hasToken) { error = WebProtocolError::INVALID_ARGUMENT; return false; }
      out.command.kind = CommandKind::ARM;
      return true;

    case WebMessageType::DISARM:
      if (!out.hasRequestId || !out.hasToken) { error = WebProtocolError::INVALID_ARGUMENT; return false; }
      out.command.kind = CommandKind::DISARM;
      return true;

    case WebMessageType::PERSONA_NEXT:
      if (!out.hasRequestId || !out.hasToken) { error = WebProtocolError::INVALID_ARGUMENT; return false; }
      out.command.kind = CommandKind::NEXT_PERSONA;
      return true;

    case WebMessageType::MOVE:
      if (!out.hasRequestId || !out.hasToken) { error = WebProtocolError::INVALID_ARGUMENT; return false; }
      out.command.driveMode = parseDriveMode(doc["mode"]);
      
      if (out.command.driveMode == DriveMode::STOPPED) {
        if (doc["mode"].is<const char*>() && strcmp(doc["mode"].as<const char*>(), "stop") == 0) {
          out.command.kind = CommandKind::STOP;
        } else {
          error = WebProtocolError::INVALID_ARGUMENT;
          return false;
        }
      } else {
        out.command.kind = CommandKind::MOVE;
      }
      
      if (doc["durationMs"].is<uint16_t>()) {
        out.command.durationMs = doc["durationMs"].as<uint16_t>();
        if (out.command.durationMs < 50 || out.command.durationMs > SAFE_DRIVE_TIME_MS) {
          error = WebProtocolError::INVALID_ARGUMENT;
          return false;
        }
      } else {
        out.command.durationMs = 0;
      }
      return true;

    case WebMessageType::ACTION:
      if (!out.hasRequestId || !out.hasToken) { error = WebProtocolError::INVALID_ARGUMENT; return false; }
      out.command.kind = CommandKind::ACTION;
      out.command.action = parseAction(doc["action"]);
      if (out.command.action == ActionId::NONE) {
        error = WebProtocolError::INVALID_ARGUMENT;
        return false;
      }
      return true;

    case WebMessageType::MOOD:
      if (!out.hasRequestId || !out.hasToken) { error = WebProtocolError::INVALID_ARGUMENT; return false; }
      out.command.kind = CommandKind::SET_MOOD;
      out.command.mood = parseMood(doc["mood"]);
      return true;

    case WebMessageType::ACCESSORY:
      if (!out.hasRequestId || !out.hasToken) { error = WebProtocolError::INVALID_ARGUMENT; return false; }
      out.command.kind = CommandKind::ACCESSORY;
      if (doc["index"].is<uint8_t>() && doc["active"].is<bool>()) {
        out.command.index = doc["index"].as<uint8_t>();
        out.command.flag = doc["active"].as<bool>();
        if (out.command.index >= 1 && out.command.index <= 3) return true;
      }
      error = WebProtocolError::INVALID_ARGUMENT;
      return false;

    default:
      error = WebProtocolError::UNKNOWN_TYPE;
      return false;
  }
}

DriveMode WebControlProtocol::parseDriveMode(const char* str) {
  if (!str) return DriveMode::STOPPED;
  if (strcmp(str, "forward") == 0) return DriveMode::FORWARD;
  if (strcmp(str, "reverse") == 0) return DriveMode::REVERSE;
  if (strcmp(str, "turn_left") == 0 || strcmp(str, "left") == 0) return DriveMode::TURN_LEFT;
  if (strcmp(str, "turn_right") == 0 || strcmp(str, "right") == 0) return DriveMode::TURN_RIGHT;
  return DriveMode::STOPPED;
}

ActionId WebControlProtocol::parseAction(const char* str) {
  if (!str) return ActionId::NONE;
  if (strcmp(str, "wave") == 0) return ActionId::WAVE;
  if (strcmp(str, "look_left") == 0) return ActionId::LOOK_LEFT;
  if (strcmp(str, "look_right") == 0) return ActionId::LOOK_RIGHT;
  if (strcmp(str, "greet") == 0) return ActionId::GREET;
  if (strcmp(str, "celebrate") == 0) return ActionId::CELEBRATE;
  if (strcmp(str, "dance") == 0) return ActionId::DANCE;
  if (strcmp(str, "sleep") == 0) return ActionId::SLEEP;
  return ActionId::NONE;
}

Mood WebControlProtocol::parseMood(const char* str) {
  if (!str) return Mood::IDLE;
  if (strcmp(str, "happy") == 0) return Mood::HAPPY;
  if (strcmp(str, "curious") == 0) return Mood::CURIOUS;
  if (strcmp(str, "sleepy") == 0) return Mood::SLEEPY;
  if (strcmp(str, "excited") == 0) return Mood::EXCITED;
  if (strcmp(str, "alert") == 0) return Mood::ALERT;
  return Mood::IDLE;
}

String WebControlProtocol::generateAck(uint32_t msgId, bool ok, const String& codeOrMessage, uint32_t revision) {
  JsonDocument doc;
  doc["type"] = "ack";
  if (msgId > 0) doc["id"] = msgId;
  doc["ok"] = ok;
  
  if (ok) {
    doc["revision"] = revision;
  } else {
    doc["code"] = codeOrMessage;
    doc["message"] = "Request rejected";
    doc["revision"] = revision;
  }
  
  String out;
  serializeJson(doc, out);
  return out;
}

String WebControlProtocol::generateError(uint32_t msgId, const String& code) {
  JsonDocument doc;
  doc["type"] = "error";
  if (msgId > 0) doc["id"] = msgId;
  doc["code"] = code;
  String out;
  serializeJson(doc, out);
  return out;
}

String WebControlProtocol::generateTelemetry(const RobotTelemetry& t) {
  JsonDocument doc;
  doc["type"] = "telemetry";
  doc["revision"] = t.revision;
  doc["uptimeMs"] = t.uptimeMs;
  doc["wifiApRunning"] = t.wifiApRunning;
  doc["wifiClientCount"] = t.wifiClientCount;
  doc["controllerPresent"] = t.controllerPresent;
  doc["pairingAvailable"] = t.pairingAvailable;
  doc["motorsArmed"] = t.motorsArmed;
  doc["motorAllowedByFirmware"] = t.motorAllowedByFirmware;
  doc["driveMode"] = static_cast<int>(t.driveMode);
  if (t.driveModeName) doc["driveModeName"] = t.driveModeName;
  doc["action"] = static_cast<int>(t.action);
  if (t.actionName) doc["actionName"] = t.actionName;
  doc["mood"] = static_cast<int>(t.mood);
  if (t.moodName) doc["moodName"] = t.moodName;
  
  if (t.rangeValid) {
    doc["rangeMm"] = t.rangeMm;
  } else {
    doc["rangeMm"] = (char*)nullptr;
  }
  
  doc["rangeValid"] = t.rangeValid;
  doc["obstacleDetected"] = t.obstacleDetected;
  doc["autonomyEnabled"] = t.autonomyEnabled;
  doc["buildName"] = t.buildName;
  doc["personaName"] = t.personaName;
  if (t.firmwareVersion) doc["firmwareVersion"] = t.firmwareVersion;
  if (t.firmwareChannel) doc["firmwareChannel"] = t.firmwareChannel;
  if (t.lastSafetyStopMs > 0) {
    doc["lastSafetyStopMs"] = t.lastSafetyStopMs;
    doc["lastSafetyStopReason"] = t.lastSafetyStopReason;
  }
  
  doc["hasDrive"] = t.hasDrive;
  doc["fourWheelDrive"] = t.fourWheelDrive;
  doc["hasManipulators"] = t.hasManipulators;
  doc["actionRunning"] = t.actionRunning;
  
  if (t.autonomyMode) doc["autonomyMode"] = t.autonomyMode;
  if (t.autonomyState) doc["autonomyState"] = t.autonomyState;
  if (t.obstacleSafetyState) doc["obstacleSafetyState"] = t.obstacleSafetyState;
  if (t.rangeSensorHealth) doc["rangeSensorHealth"] = t.rangeSensorHealth;
  doc["forwardMotionBlocked"] = t.forwardMotionBlocked;
  doc["actionRunning"] = t.actionRunning;

  String out;
  serializeJson(doc, out);
  return out;
}

String WebControlProtocol::generateEventLog(const EventLogEntry* entries, size_t count) {
  JsonDocument doc;
  doc["type"] = "events";
  JsonArray arr = doc["events"].to<JsonArray>();
  for (size_t i = 0; i < count; i++) {
    JsonObject obj = arr.add<JsonObject>();
    obj["ts"] = entries[i].timestampMs;
    obj["sev"] = entries[i].severity;
    obj["code"] = entries[i].code;
  }
  String out;
  serializeJson(doc, out);
  return out;
}
