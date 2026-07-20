#pragma once
#include "ControlTypes.h"
#include "RobotAPI.h"
#include "SystemStatus.h"

class ControlRouter {
public:
  void begin(RobotAPI* robot, SystemStatus* status);
  bool execute(const RobotCommand& cmd);

  uint32_t currentEpoch() const { return _commandEpoch; }
  uint32_t lastInterventionEpoch() const { return _lastInterventionEpoch; }

private:
  RobotAPI* _robot = nullptr;
  SystemStatus* _status = nullptr;

  uint32_t _commandEpoch = 1;
  uint32_t _lastInterventionEpoch = 0;
};
