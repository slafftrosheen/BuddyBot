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

  if (op == "EXPR") {
    String expr = tokenAt(line, 2, '|');
    out.kind = CommandKind::PLAY_EXPRESSION;
    
    if (expr == "NONE") out.expression = ExpressionId::NONE;
    else if (expr == "SURPRISED") out.expression = ExpressionId::SURPRISED;
    else if (expr == "CONFUSED") out.expression = ExpressionId::CONFUSED;
    else if (expr == "WORRIED") out.expression = ExpressionId::WORRIED;
    else if (expr == "SCARED") out.expression = ExpressionId::SCARED;
    else if (expr == "GIGGLE") out.expression = ExpressionId::GIGGLE;
    else if (expr == "LOVE") out.expression = ExpressionId::LOVE;
    else if (expr == "PROUD") out.expression = ExpressionId::PROUD;
    else if (expr == "SLEEPYAWN") out.expression = ExpressionId::SLEEP_YAWN;
    else if (expr == "THINK") out.expression = ExpressionId::THINK;
    else if (expr == "LISTEN") out.expression = ExpressionId::LISTEN;
    else if (expr == "OBSTACLE") out.expression = ExpressionId::OBSTACLE;
    else if (expr == "WINKLEFT") out.expression = ExpressionId::WINK_LEFT;
    else if (expr == "WINKRIGHT") out.expression = ExpressionId::WINK_RIGHT;
    else return false;
    
    return true;
  }
  
  if (op == "ATTENTION") {
    String tgt = tokenAt(line, 2, '|');
    out.kind = CommandKind::SET_ATTENTION;
    
    if (tgt == "CENTER") out.attention = AttentionTarget::CENTER;
    else if (tgt == "LEFT") out.attention = AttentionTarget::LEFT;
    else if (tgt == "RIGHT") out.attention = AttentionTarget::RIGHT;
    else if (tgt == "UP") out.attention = AttentionTarget::UP;
    else if (tgt == "DOWN") out.attention = AttentionTarget::DOWN;
    else if (tgt == "RANDOM") out.attention = AttentionTarget::RANDOM;
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

  if (op == "VERSION") {
    out.kind = CommandKind::VERSION;
    return true;
  }

  if (op == "AUTONOMY") {
    out.kind = CommandKind::AUTONOMY_SET;
    String val = tokenAt(line, 2, '|');
    out.flag = (val == "ON");
    return true;
  }

  if (op == "WIFI") {
    String sub = tokenAt(line, 2, '|');
    if (sub == "STATUS") { out.kind = CommandKind::CTRL_WIFI_STATUS; return true; }
    if (sub == "ON") { out.kind = CommandKind::CTRL_WIFI_ON; return true; }
    if (sub == "OFF") { out.kind = CommandKind::CTRL_WIFI_OFF; return true; }
    if (sub == "PAIR") { out.kind = CommandKind::CTRL_WIFI_PAIR; return true; }
    return false;
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
  if (line == "VERSION") { out.kind = CommandKind::VERSION; return true; }
  if (line == "DIAG BOOT") { out.kind = CommandKind::DIAG_BOOT; return true; }
  if (line == "EVENTS") { out.kind = CommandKind::PRINT_EVENTS; return true; }
  if (line == "SENSOR STATUS") { out.kind = CommandKind::CTRL_SENSOR_STATUS; return true; }
  if (line == "SAFETY STATUS") { out.kind = CommandKind::CTRL_SAFETY_STATUS; return true; }
  if (line == "AUTONOMY STATUS") { out.kind = CommandKind::CTRL_AUTONOMY_STATUS; return true; }
  if (line == "AUTONOMY ON") { out.kind = CommandKind::AUTONOMY_SET; out.flag = true; return true; }
  if (line == "AUTONOMY OFF") { out.kind = CommandKind::AUTONOMY_SET; out.flag = false; return true; }
  
  if (line == "WIFI STATUS") { out.kind = CommandKind::CTRL_WIFI_STATUS; return true; }
  if (line == "WIFI ON") { out.kind = CommandKind::CTRL_WIFI_ON; return true; }
  if (line == "WIFI OFF") { out.kind = CommandKind::CTRL_WIFI_OFF; return true; }
  if (line == "WIFI PAIR") { out.kind = CommandKind::CTRL_WIFI_PAIR; return true; }

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

  if (line.startsWith("EXPR ")) {
    out.kind = CommandKind::PLAY_EXPRESSION;
    if (line == "EXPR NONE") { out.expression = ExpressionId::NONE; return true; }
    if (line == "EXPR SURPRISED") { out.expression = ExpressionId::SURPRISED; return true; }
    if (line == "EXPR CONFUSED") { out.expression = ExpressionId::CONFUSED; return true; }
    if (line == "EXPR WORRIED") { out.expression = ExpressionId::WORRIED; return true; }
    if (line == "EXPR SCARED") { out.expression = ExpressionId::SCARED; return true; }
    if (line == "EXPR GIGGLE") { out.expression = ExpressionId::GIGGLE; return true; }
    if (line == "EXPR LOVE") { out.expression = ExpressionId::LOVE; return true; }
    if (line == "EXPR PROUD") { out.expression = ExpressionId::PROUD; return true; }
    if (line == "EXPR SLEEPYAWN") { out.expression = ExpressionId::SLEEP_YAWN; return true; }
    if (line == "EXPR THINK") { out.expression = ExpressionId::THINK; return true; }
    if (line == "EXPR LISTEN") { out.expression = ExpressionId::LISTEN; return true; }
    if (line == "EXPR OBSTACLE") { out.expression = ExpressionId::OBSTACLE; return true; }
    if (line == "EXPR WINKLEFT") { out.expression = ExpressionId::WINK_LEFT; return true; }
    if (line == "EXPR WINKRIGHT") { out.expression = ExpressionId::WINK_RIGHT; return true; }
  }

  if (line.startsWith("ATTENTION ")) {
    out.kind = CommandKind::SET_ATTENTION;
    if (line == "ATTENTION CENTER") { out.attention = AttentionTarget::CENTER; return true; }
    if (line == "ATTENTION LEFT") { out.attention = AttentionTarget::LEFT; return true; }
    if (line == "ATTENTION RIGHT") { out.attention = AttentionTarget::RIGHT; return true; }
    if (line == "ATTENTION UP") { out.attention = AttentionTarget::UP; return true; }
    if (line == "ATTENTION DOWN") { out.attention = AttentionTarget::DOWN; return true; }
    if (line == "ATTENTION RANDOM") { out.attention = AttentionTarget::RANDOM; return true; }
  }

  if (line == "ACC 1 ON") { out.kind = CommandKind::ACCESSORY; out.index = 1; out.flag = true; return true; }
  if (line == "ACC 1 OFF") { out.kind = CommandKind::ACCESSORY; out.index = 1; out.flag = false; return true; }
  if (line == "ACC 2 ON") { out.kind = CommandKind::ACCESSORY; out.index = 2; out.flag = true; return true; }
  if (line == "ACC 2 OFF") { out.kind = CommandKind::ACCESSORY; out.index = 2; out.flag = false; return true; }
  if (line == "ACC 3 ON") { out.kind = CommandKind::ACCESSORY; out.index = 3; out.flag = true; return true; }
  if (line == "ACC 3 OFF") { out.kind = CommandKind::ACCESSORY; out.index = 3; out.flag = false; return true; }

  if (line.startsWith("SERVO TEST ")) {
    out.kind = CommandKind::SERVO_TEST;
    String sub = line.substring(11);
    if (sub == "UNLOCK") { out.arg1 = -1; return true; }
    if (sub == "STOP") { out.arg1 = -2; return true; }
    
    // SERVO TEST <0-7> <speed>
    int space = sub.indexOf(' ');
    if (space > 0) {
      out.arg1 = sub.substring(0, space).toInt();
      out.arg2 = sub.substring(space + 1).toInt();
      return true;
    }
  }

  if (line.startsWith("JOINT MOVE ")) {
    out.kind = CommandKind::JOINT_MOVE;
    String sub = line.substring(11); // "<role> <deg> <dur>"
    int space1 = sub.indexOf(' ');
    if (space1 > 0) {
      out.arg1 = sub.substring(0, space1).toInt();
      String rest = sub.substring(space1 + 1);
      int space2 = rest.indexOf(' ');
      if (space2 > 0) {
        out.arg2 = rest.substring(0, space2).toInt(); // deg
        out.durationMs = rest.substring(space2 + 1).toInt();
      } else {
        out.arg2 = rest.toInt();
        out.durationMs = 500;
      }
      return true;
    }
  }

  if (line.startsWith("JOINT REST ")) {
    out.kind = CommandKind::JOINT_REST;
    out.arg1 = line.substring(11).toInt();
    return true;
  }

  return false;
}

void ControlProtocol::printHelp() const {
  Serial.println("Legacy commands:");
  Serial.println("ARM DISARM STOP");
  Serial.println("MOOD IDLE|HAPPY|CURIOUS|SLEEPY|EXCITED|ALERT");
  Serial.println("PERSONA NEXT");
  Serial.println("MOVE FWD|REV|LEFT|RIGHT");
  Serial.println("ACTION WAVE|GREET|LOOKLEFT|LOOKRIGHT|CELEBRATE|DANCE|SLEEP");
  Serial.println("EXPR NONE|SURPRISED|CONFUSED|WORRIED|SCARED|GIGGLE|LOVE|PROUD|SLEEPYAWN|THINK|LISTEN|OBSTACLE|WINKLEFT|WINKRIGHT");
  Serial.println("ATTENTION CENTER|LEFT|RIGHT|UP|DOWN|RANDOM");
  Serial.println("ACC 1|2|3 ON|OFF");
  Serial.println("RANGE");
  Serial.println("PROFILE LIST");
  Serial.println("PROFILE SHOW");
  Serial.println("STATUS");
  Serial.println("AUTONOMY ON|OFF");
  Serial.println("WIFI STATUS|ON|OFF|PAIR");

  Serial.println("Pipe protocol:");
  Serial.println("CMD|ARM");
  Serial.println("CMD|MOVE|FWD|500");
  Serial.println("CMD|ACTION|WAVE");
  Serial.println("CMD|EXPR|GIGGLE");
  Serial.println("CMD|ATTENTION|LEFT");
  Serial.println("CMD|PROFILE|SHOW");
  Serial.println("CMD|STATUS");
  Serial.println("CMD|AUTONOMY|ON");
  Serial.println("CMD|WIFI|STATUS");
}
