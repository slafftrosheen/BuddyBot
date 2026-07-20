#include "WebControlProtocol.h"

WebControlProtocol::WebControlProtocol() {}

bool WebControlProtocol::parseCommand(const char* jsonStr, RobotCommand& outCmd, bool& isAuthCmd, String& outPin, uint32_t& outMsgId) {
  isAuthCmd = false;
  outCmd.kind = CommandKind::NONE;
  outCmd.source = ControlSource::WIFI;
  outMsgId = 0;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, jsonStr);
  if (err) return false;

  if (doc["id"].is<uint32_t>()) outMsgId = doc["id"].as<uint32_t>();
  
  const char* type = doc["type"];
  if (!type) return false;

  String t = type;
  if (t == "auth") {
    isAuthCmd = true;
    outPin = doc["pin"].as<String>();
    return true;
  } else if (t == "stop") {
    outCmd.kind = CommandKind::STOP;
    return true;
  } else if (t == "arm") {
    outCmd.kind = CommandKind::ARM;
    return true;
  } else if (t == "disarm") {
    outCmd.kind = CommandKind::DISARM;
    return true;
  } else if (t == "move") {
    outCmd.kind = CommandKind::MOVE;
    outCmd.driveMode = parseDriveMode(doc["mode"]);
    outCmd.durationMs = doc["durationMs"].as<uint16_t>();
    return true;
  } else if (t == "action") {
    outCmd.kind = CommandKind::ACTION;
    outCmd.action = parseAction(doc["action"]);
    return true;
  } else if (t == "mood") {
    outCmd.kind = CommandKind::SET_MOOD;
    outCmd.mood = parseMood(doc["mood"]);
    return true;
  } else if (t == "accessory") {
    outCmd.kind = CommandKind::ACCESSORY;
    outCmd.index = doc["index"].as<uint8_t>();
    outCmd.flag = doc["active"].as<bool>();
    return true;
  }
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

String WebControlProtocol::generateAck(uint32_t msgId, bool ok, const String& message, bool isArmed, int authFlag) {
  JsonDocument doc;
  doc["type"] = "ack";
  doc["id"] = msgId;
  doc["ok"] = ok;
  if (message.length() > 0) doc["message"] = message;
  doc["armed"] = isArmed;
  if (authFlag != -1) {
    doc["auth"] = (authFlag == 1);
  }
  String out;
  serializeJson(doc, out);
  return out;
}

String WebControlProtocol::generateTelemetry(const TelemetryData& t) {
  JsonDocument doc;
  doc["type"] = "telemetry";
  doc["isConnected"] = t.isConnected;
  doc["isArmed"] = t.isArmed;
  doc["mood"] = static_cast<int>(t.mood);
  doc["driveMode"] = static_cast<int>(t.driveMode);
  doc["actionId"] = static_cast<int>(t.actionId);
  doc["rangeMm"] = t.rangeMm;
  doc["hasController"] = t.hasController;
  
  String out;
  serializeJson(doc, out);
  return out;
}
