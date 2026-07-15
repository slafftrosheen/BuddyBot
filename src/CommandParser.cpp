#include "CommandParser.h"

bool CommandParser::parse(String command, RobotCommand& out) {
  return _protocol.parseLine(command, out);
}
