#pragma once
#include "ControlTypes.h"
#include "RobotAPI.h"

enum class AutonomyState : uint8_t {
  IDLE,
  MONITORING,
  STOPPING,
  BACKING_UP,
  TURNING,
  WAIT_FOR_CLEARANCE,
  RESUMING
};

class AutonomyManager {
public:
  void begin(RobotAPI* robot);

  void setEnabled(bool enabled);
  bool enabled() const;

  bool update(RobotCommand& outCommand);
  AutonomyState state() const { return _state; }

private:
  RobotAPI* _robot = nullptr;
  bool _enabled = false;
  AutonomyState _state = AutonomyState::IDLE;
  uint32_t _stateMs = 0;
};
