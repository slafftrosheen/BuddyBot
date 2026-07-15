#pragma once
#include "ControlTypes.h"
#include "RobotAPI.h"

class AutonomyManager {
public:
  void begin(RobotAPI* robot);

  void setEnabled(bool enabled);
  bool enabled() const;

  bool update(RobotCommand& outCommand);

private:
  RobotAPI* _robot = nullptr;
  bool _enabled = false;
  uint32_t _cooldownUntilMs = 0;
};
