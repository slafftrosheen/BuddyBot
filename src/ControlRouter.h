#pragma once
#include "ControlTypes.h"
#include "RobotAPI.h"
#include "SystemStatus.h"

class ControlRouter {
public:
  void begin(RobotAPI* robot, SystemStatus* status);
  bool execute(const RobotCommand& cmd);

private:
  RobotAPI* _robot = nullptr;
  SystemStatus* _status = nullptr;
};
