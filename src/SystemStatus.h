#pragma once
#include "RobotAPI.h"
#include "RobotHal.h"

class SystemStatus {
public:
  void begin(RobotAPI* robot, RobotHal* hal);
  void printStatus() const;

private:
  RobotAPI* _robot = nullptr;
  RobotHal* _hal = nullptr;
};
