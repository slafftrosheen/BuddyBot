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
  static bool hasTokenCount(const String& input, char sep, int expectedCount);
  static bool parseBoundedUInt(const String& input, uint16_t minimum, uint16_t maximum, uint16_t& out);
};
