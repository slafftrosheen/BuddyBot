#pragma once
#include "ControlProtocol.h"

class CommandParser {
public:
  bool parse(String command, RobotCommand& out);

private:
  ControlProtocol _protocol;
};
