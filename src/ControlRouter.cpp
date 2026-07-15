#include "ControlRouter.h"
#include "BuildProfiles.h"

void ControlRouter::begin(RobotAPI* robot, SystemStatus* status) {
  _robot = robot;
  _status = status;
}

bool ControlRouter::execute(const RobotCommand& cmd) {
  if (_robot == nullptr) return false;

  switch (cmd.kind) {
    case CommandKind::ARM:
      _robot->armMotors();
      return true;

    case CommandKind::DISARM:
      _robot->disarmMotors();
      return true;

    case CommandKind::STOP:
      _robot->stopAll();
      return true;

    case CommandKind::SET_MOOD:
      _robot->setMood(cmd.mood);
      return true;

    case CommandKind::NEXT_PERSONA:
      _robot->nextPersona();
      return true;

    case CommandKind::MOVE:
      _robot->move(cmd.driveMode, cmd.durationMs);
      return true;

    case CommandKind::ACTION:
      _robot->action(cmd.action);
      return true;

    case CommandKind::ACCESSORY:
      _robot->accessory(cmd.index, cmd.flag);
      return true;

    case CommandKind::RANGE_QUERY: {
      RangeReading r = _robot->rangeReading();
      if (r.valid) {
        Serial.printf("RANGE %umm\n", r.distanceMm);
      } else {
        Serial.println("RANGE INVALID");
      }
      return true;
    }

    case CommandKind::PROFILE_LIST:
      printAllBuildProfiles();
      return true;

    case CommandKind::PROFILE_SHOW:
      Serial.printf("active_profile=%s\n", getActiveBuildName());
      printBuildProfile(_robot->buildConfig());
      return true;

    case CommandKind::STATUS_QUERY:
      if (_status) _status->printStatus();
      return true;

    case CommandKind::AUTONOMY_SET:
      _robot->setAutonomyEnabled(cmd.flag);
      Serial.printf("autonomy=%s\n", cmd.flag ? "ON" : "OFF");
      return true;

    default:
      return false;
  }
}
