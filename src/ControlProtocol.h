#pragma once
#include <Arduino.h>
#include "ControlTypes.h"

class ControlProtocol {
public:
  bool parseLine(String line, RobotCommand& out);
  void printHelp() const;

private:
  bool parsePipeCommand(String line, RobotCommand& out);
  bool parseLegacyCommand(String line, RobotCommand& out);

  static String tokenAt(const String& input, int index, char sep);
};
