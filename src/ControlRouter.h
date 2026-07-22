#pragma once
#include "ControlTypes.h"
#include "RobotAPI.h"
#include "SystemStatus.h"

class WifiControl;

class ControlRouter {
public:
  void begin(RobotAPI* robot, SystemStatus* status);
  void setDriveBase(IDriveBase* drive) { _drive = drive; }
  void setWifiControl(WifiControl* wifi) { _wifi = wifi; }
  
  bool execute(const RobotCommand& cmd);
  
  uint32_t currentEpoch() const { return _currentEpoch; }
  uint32_t lastInterventionEpoch() const { return _lastInterventionEpoch; }

private:
  RobotAPI* _robot = nullptr;
  SystemStatus* _status = nullptr;
  IDriveBase* _drive = nullptr;
  WifiControl* _wifi = nullptr;
  
  uint32_t _currentEpoch = 1;
  uint32_t _lastInterventionEpoch = 0;
};
