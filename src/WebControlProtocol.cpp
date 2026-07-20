#include "WebControlProtocol.h"
#include "Config.h"

WebControlProtocol::WebControlProtocol() {}

bool WebControlProtocol::parseCommand(
  const char* jsonStr, size_t len,
  RobotCommand& outCmd,
  String& outType,
  String& outToken,
  String& outCode,
  uint32_t& outMsgId,
  String& outErrorCode
) {
  outCmd.kind = CommandKind::NONE;
  outCmd.source = ControlSource::WIFI;
  outMsgId = 0;
  outType = "";
  outToken = "";
  outCode = "";
  outErrorCode = "bad_json";

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, jsonStr, len);
  if (err) return false;

  if (!doc["v"].is<int>() || doc["v"].as<int>() != 1) {
    outErrorCode = "bad_json";
    return false;
  }

  if (doc["id"].is<uint32_t>()) outMsgId = doc["id"].as<uint32_t>();
  
  const char* type = doc["type"];
  if (!type) return false;
  
  outType = type;

  if (doc["token"].is<const char*>()) {
    outToken = doc["token"].as<const char*>();
  }

  if (outType == "hello" || outType == "ping" || outType == "status") {
    return true;
  } else if (outType == "pair") {
    if (doc["code"].is<const char*>()) {
      outCode = doc["code"].as<const char*>();
      return true;
    }
    outErrorCode = "invalid_argument";
    return false;
  } else if (outType == "stop") {
    outCmd.kind = CommandKind::STOP;
    return true;
  } else if (outType == "arm") {
    outCmd.kind = CommandKind::ARM;
    return true;
  } else if (outType == "disarm") {
    outCmd.kind = CommandKind::DISARM;
    return true;
  } else if (outType == "move") {
    outCmd.kind = CommandKind::MOVE;
    outCmd.driveMode = parseDriveMode(doc["mode"]);
    if (outCmd.driveMode == DriveMode::STOPPED && String(doc["mode"].as<const char*>()) != "stop") {
      outErrorCode = "invalid_argument";
      return false;
    }
    
    if (doc["durationMs"].is<uint16_t>()) {
      outCmd.durationMs = doc["durationMs"].as<uint16_t>();
      if (outCmd.durationMs < 50 || outCmd.durationMs > SAFE_DRIVE_TIME_MS) {
        outErrorCode = "invalid_argument";
        return false;
      }
    } else {
      outCmd.durationMs = 0;
    }
    return true;
  } else if (outType == "action") {
    outCmd.kind = CommandKind::ACTION;
    outCmd.action = parseAction(doc["action"]);
    if (outCmd.action == ActionId::NONE) {
      outErrorCode = "invalid_argument";
      return false;
    }
    return true;
  } else if (outType == "mood") {
    outCmd.kind = CommandKind::SET_MOOD;
    outCmd.mood = parseMood(doc["mood"]);
    return true;
  } else if (outType == "persona_next") {
    outCmd.kind = CommandKind::NEXT_PERSONA;
    return true;
  } else if (outType == "accessory") {
    outCmd.kind = CommandKind::ACCESSORY;
    if (doc["index"].is<uint8_t>() && doc["active"].is<bool>()) {
      outCmd.index = doc["index"].as<uint8_t>();
      outCmd.flag = doc["active"].as<bool>();
      if (outCmd.index >= 1 && outCmd.index <= 3) return true;
    }
    outErrorCode = "invalid_argument";
    return false;
  }
  
  outErrorCode = "bad_json";
  return false;
}

DriveMode WebControlProtocol::parseDriveMode(const char* str) {
  if (!str) return DriveMode::STOPPED;
  String s = str;
  if (s == "forward") return DriveMode::FORWARD;
  if (s == "reverse") return DriveMode::REVERSE;
  if (s == "turn_left") return DriveMode::TURN_LEFT;
  if (s == "turn_right") return DriveMode::TURN_RIGHT;
  return DriveMode::STOPPED;
}

ActionId WebControlProtocol::parseAction(const char* str) {
  if (!str) return ActionId::NONE;
  String s = str;
  if (s == "wave") return ActionId::WAVE;
  if (s == "look_left") return ActionId::LOOK_LEFT;
  if (s == "look_right") return ActionId::LOOK_RIGHT;
  if (s == "greet") return ActionId::GREET;
  if (s == "celebrate") return ActionId::CELEBRATE;
  if (s == "dance") return ActionId::DANCE;
  if (s == "sleep") return ActionId::SLEEP;
  return ActionId::NONE;
}

Mood WebControlProtocol::parseMood(const char* str) {
  if (!str) return Mood::IDLE;
  String s = str;
  if (s == "happy") return Mood::HAPPY;
  if (s == "curious") return Mood::CURIOUS;
  if (s == "sleepy") return Mood::SLEEPY;
  if (s == "excited") return Mood::EXCITED;
  if (s == "alert") return Mood::ALERT;
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
  doc["motorsArmed"] = t.motorsArmed;
  doc["motorAllowedByFirmware"] = t.motorAllowedByFirmware;
  doc["driveMode"] = static_cast<int>(t.driveMode);
  doc["action"] = static_cast<int>(t.action);
  doc["mood"] = static_cast<int>(t.mood);
  doc["rangeMm"] = t.rangeMm;
  doc["rangeValid"] = t.rangeValid;
  doc["obstacleDetected"] = t.obstacleDetected;
  doc["autonomyEnabled"] = t.autonomyEnabled;
  doc["buildName"] = t.buildName;
  doc["personaName"] = t.personaName;
  doc["lastSafetyStopMs"] = t.lastSafetyStopMs;
  doc["lastSafetyStopReason"] = t.lastSafetyStopReason;
  
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
