#pragma once
#include "ControlTypes.h"
#include "RobotAPI.h"
#include "SystemStatus.h"
#include "SafetySupervisor.h"

class WifiControl;
class AutonomyManager;

class ControlRouter {
public:
  void begin(RobotAPI* robot, SystemStatus* status, SafetySupervisor* safety);
  void setDriveBase(IDriveBase* drive) { _drive = drive; }
  void setWifiControl(WifiControl* wifi) { _wifi = wifi; }
  void setAutonomyManager(AutonomyManager* autonomy) { _autonomy = autonomy; }

  void completeBoot();
  void updateSafety();
  void emergencyStop(SafetyFault fault);
  void physicalEmergencyStop();
  bool clearPhysicalEmergencyStop();

  bool execute(const RobotCommand& cmd);
  
  uint32_t currentEpoch() const { return _currentEpoch; }
  uint32_t lastInterventionEpoch() const { return _lastInterventionEpoch; }
  SafetyState safetyState() const;
  SafetyFault safetyFault() const;

private:
  SafetyInputs buildSafetyInputs() const;
  void disableAutonomy();
  void recordSafetyTransition();

  RobotAPI* _robot = nullptr;
  SystemStatus* _status = nullptr;
  IDriveBase* _drive = nullptr;
  WifiControl* _wifi = nullptr;
  AutonomyManager* _autonomy = nullptr;
  SafetySupervisor* _safety = nullptr;
  
  uint32_t _currentEpoch = 1;
  uint32_t _lastInterventionEpoch = 0;
  bool _bootComplete = false;
  SafetyState _lastReportedSafetyState = SafetyState::BOOT;
  SafetyFault _lastReportedSafetyFault = SafetyFault::NONE;
};
