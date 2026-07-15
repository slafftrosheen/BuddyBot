#include "ControlProtocol.h"

String ControlProtocol::tokenAt(const String& input, int index, char sep) {
  int start = 0;
  int current = 0;

  for (int i = 0; i <= input.length(); ++i) {
    if (i == input.length() || input[i] == sep) {
      if (current == index) {
        return input.substring(start, i);
      }
      current++;
      start = i + 1;
    }
  }
  return "";
}

bool ControlProtocol::parseLine(String line, RobotCommand& out) {
  line.trim();
  if (line.length() == 0) return false;

  if (line.indexOf('|') >= 0) {
    return parsePipeCommand(line, out);
  }

  return parseLegacyCommand(line, out);
}

bool ControlProtocol::parsePipeCommand(String line, RobotCommand& out) {
  line.trim();
  line.toUpperCase();
  out = {};

  String head = tokenAt(line, 0, '|');
  if (head != "CMD") return false;

  String op = tokenAt(line, 1, '|');

  if (op == "ARM") {
    out.kind = CommandKind::ARM;
    return true;
  }

  if (op == "DISARM") {
    out.kind = CommandKind::DISARM;
    return true;
  }

  if (op == "STOP") {
    out.kind = CommandKind::STOP;
    return true;
  }

  if (op == "MOOD") {
    String mood = tokenAt(line, 2, '|');
    out.kind = CommandKind::SET_MOOD;
    if (mood == "IDLE") out.mood = Mood::IDLE;
    else if (mood == "HAPPY") out.mood = Mood::HAPPY;
    else if (mood == "CURIOUS") out.mood = Mood::CURIOUS;
    else if (mood == "SLEEPY") out.mood = Mood::SLEEPY;
    else if (mood == "EXCITED") out.mood = Mood::EXCITED;
    else if (mood == "ALERT") out.mood = Mood::ALERT;
    else return false;
    return true;
  }

  if (op == "PERSONA" && tokenAt(line, 2, '|') == "NEXT") {
    out.kind = CommandKind::NEXT_PERSONA;
    return true;
  }

  if (op == "MOVE") {
    String dir = tokenAt(line, 2, '|');
    String dur = tokenAt(line, 3, '|');

    out.kind = CommandKind::MOVE;
    out.durationMs = dur.length() ? uint16_t(dur.toInt()) : 500;

    if (dir == "FWD") out.driveMode = DriveMode::FORWARD;
    else if (dir == "REV") out.driveMode = DriveMode::REVERSE;
    else if (dir == "LEFT") out.driveMode = DriveMode::TURN_LEFT;
    else if (dir == "RIGHT") out.driveMode = DriveMode::TURN_RIGHT;
    else return false;

    return true;
  }

  if (op == "ACTION") {
    String act = tokenAt(line, 2, '|');
    out.kind = CommandKind::ACTION;

    if (act == "WAVE") out.action = ActionId::WAVE;
    else if (act == "GREET") out.action = ActionId::GREET;
    else if (act == "LOOKLEFT") out.action = ActionId::LOOK_LEFT;
    else if (act == "LOOKRIGHT") out.action = ActionId::LOOK_RIGHT;
    else if (act == "CELEBRATE") out.action = ActionId::CELEBRATE;
    else if (act == "DANCE") out.action = ActionId::DANCE;
    else if (act == "SLEEP") out.action = ActionId::SLEEP;
    else return false;

    return true;
  }

  if (op == "ACC") {
    out.kind = CommandKind::ACCESSORY;

    String idx = tokenAt(line, 2, '|');
    String state = tokenAt(line, 3, '|');

    out.index = uint8_t(idx.toInt());
    out.flag = (state == "ON");

    return out.index >= 1 && out.index <= 3;
  }

  if (op == "RANGE") {
    out.kind = CommandKind::RANGE_QUERY;
    return true;
  }

  if (op == "PROFILE" && tokenAt(line, 2, '|') == "LIST") {
    out.kind = CommandKind::PROFILE_LIST;
    return true;
  }

  if (op == "PROFILE" && tokenAt(line, 2, '|') == "SHOW") {
    out.kind = CommandKind::PROFILE_SHOW;
    return true;
  }

  if (op == "STATUS") {
    out.kind = CommandKind::STATUS_QUERY;
    return true;
  }

  if (op == "AUTONOMY") {
    out.kind = CommandKind::AUTONOMY_SET;
    String val = tokenAt(line, 2, '|');
    out.flag = (val == "ON");
    return true;
  }

  return false;
}

bool ControlProtocol::parseLegacyCommand(String line, RobotCommand& out) {
  line.trim();
  line.toUpperCase();
  out = {};

  if (line == "ARM") { out.kind = CommandKind::ARM; return true; }
  if (line == "DISARM") { out.kind = CommandKind::DISARM; return true; }
  if (line == "STOP") { out.kind = CommandKind::STOP; return true; }
  if (line == "PERSONA NEXT") { out.kind = CommandKind::NEXT_PERSONA; return true; }
  if (line == "RANGE") { out.kind = CommandKind::RANGE_QUERY; return true; }
  if (line == "PROFILE LIST") { out.kind = CommandKind::PROFILE_LIST; return true; }
  if (line == "PROFILE SHOW") { out.kind = CommandKind::PROFILE_SHOW; return true; }
  if (line == "STATUS") { out.kind = CommandKind::STATUS_QUERY; return true; }
  if (line == "AUTONOMY ON") { out.kind = CommandKind::AUTONOMY_SET; out.flag = true; return true; }
  if (line == "AUTONOMY OFF") { out.kind = CommandKind::AUTONOMY_SET; out.flag = false; return true; }

  if (line == "MOOD IDLE") { out.kind = CommandKind::SET_MOOD; out.mood = Mood::IDLE; return true; }
  if (line == "MOOD HAPPY") { out.kind = CommandKind::SET_MOOD; out.mood = Mood::HAPPY; return true; }
  if (line == "MOOD CURIOUS") { out.kind = CommandKind::SET_MOOD; out.mood = Mood::CURIOUS; return true; }
  if (line == "MOOD SLEEPY") { out.kind = CommandKind::SET_MOOD; out.mood = Mood::SLEEPY; return true; }
  if (line == "MOOD EXCITED") { out.kind = CommandKind::SET_MOOD; out.mood = Mood::EXCITED; return true; }
  if (line == "MOOD ALERT") { out.kind = CommandKind::SET_MOOD; out.mood = Mood::ALERT; return true; }

  if (line == "MOVE FWD") { out.kind = CommandKind::MOVE; out.driveMode = DriveMode::FORWARD; out.durationMs = 500; return true; }
  if (line == "MOVE REV") { out.kind = CommandKind::MOVE; out.driveMode = DriveMode::REVERSE; out.durationMs = 500; return true; }
  if (line == "MOVE LEFT") { out.kind = CommandKind::MOVE; out.driveMode = DriveMode::TURN_LEFT; out.durationMs = 350; return true; }
  if (line == "MOVE RIGHT") { out.kind = CommandKind::MOVE; out.driveMode = DriveMode::TURN_RIGHT; out.durationMs = 350; return true; }

  if (line == "ACTION WAVE") { out.kind = CommandKind::ACTION; out.action = ActionId::WAVE; return true; }
  if (line == "ACTION GREET") { out.kind = CommandKind::ACTION; out.action = ActionId::GREET; return true; }
  if (line == "ACTION LOOKLEFT") { out.kind = CommandKind::ACTION; out.action = ActionId::LOOK_LEFT; return true; }
  if (line == "ACTION LOOKRIGHT") { out.kind = CommandKind::ACTION; out.action = ActionId::LOOK_RIGHT; return true; }
  if (line == "ACTION CELEBRATE") { out.kind = CommandKind::ACTION; out.action = ActionId::CELEBRATE; return true; }
  if (line == "ACTION DANCE") { out.kind = CommandKind::ACTION; out.action = ActionId::DANCE; return true; }
  if (line == "ACTION SLEEP") { out.kind = CommandKind::ACTION; out.action = ActionId::SLEEP; return true; }

  if (line == "ACC 1 ON") { out.kind = CommandKind::ACCESSORY; out.index = 1; out.flag = true; return true; }
  if (line == "ACC 1 OFF") { out.kind = CommandKind::ACCESSORY; out.index = 1; out.flag = false; return true; }
  if (line == "ACC 2 ON") { out.kind = CommandKind::ACCESSORY; out.index = 2; out.flag = true; return true; }
  if (line == "ACC 2 OFF") { out.kind = CommandKind::ACCESSORY; out.index = 2; out.flag = false; return true; }
  if (line == "ACC 3 ON") { out.kind = CommandKind::ACCESSORY; out.index = 3; out.flag = true; return true; }
  if (line == "ACC 3 OFF") { out.kind = CommandKind::ACCESSORY; out.index = 3; out.flag = false; return true; }

  return false;
}

void ControlProtocol::printHelp() const {
  Serial.println("Legacy commands:");
  Serial.println("ARM DISARM STOP");
  Serial.println("MOOD IDLE|HAPPY|CURIOUS|SLEEPY|EXCITED|ALERT");
  Serial.println("PERSONA NEXT");
  Serial.println("MOVE FWD|REV|LEFT|RIGHT");
  Serial.println("ACTION WAVE|GREET|LOOKLEFT|LOOKRIGHT|CELEBRATE|DANCE|SLEEP");
  Serial.println("ACC 1|2|3 ON|OFF");
  Serial.println("RANGE");
  Serial.println("PROFILE LIST");
  Serial.println("PROFILE SHOW");
  Serial.println("STATUS");
  Serial.println("AUTONOMY ON|OFF");

  Serial.println("Pipe protocol:");
  Serial.println("CMD|ARM");
  Serial.println("CMD|MOVE|FWD|500");
  Serial.println("CMD|ACTION|WAVE");
  Serial.println("CMD|PROFILE|SHOW");
  Serial.println("CMD|STATUS");
  Serial.println("CMD|AUTONOMY|ON");
}
