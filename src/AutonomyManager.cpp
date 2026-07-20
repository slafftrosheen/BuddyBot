#include "AutonomyManager.h"
#include "Config.h"

void AutonomyManager::begin(RobotAPI* robot) {
  _robot = robot;
  _state = AutonomyState::IDLE;
}

void AutonomyManager::setEnabled(bool enabled) {
  _enabled = enabled;
  if (!_enabled) {
    _state = AutonomyState::IDLE;
  }
}

bool AutonomyManager::enabled() const {
  return _enabled;
}

bool AutonomyManager::update(RobotCommand& outCommand) {
  outCommand = {}; // Initialize to NONE
  if (!_enabled || !_robot) {
    if (_state != AutonomyState::IDLE) _state = AutonomyState::IDLE;
    return false;
  }
  if (_state == AutonomyState::IDLE) {
    _state = AutonomyState::MONITORING;
  }

  RangeReading r = _robot->rangeReading();
  uint32_t now = millis();

  switch (_state) {
    case AutonomyState::MONITORING: {
      if (r.valid && r.distanceMm > 0 && r.distanceMm < OBSTACLE_STOP_MM) {
        outCommand.source = ControlSource::AUTONOMY;
        outCommand.kind = CommandKind::STOP;
        _state = AutonomyState::STOPPING;
        // RobotAPI saved the manual command before FSM executes.
        return true;
      }
      break;
    }
    case AutonomyState::STOPPING: {
      outCommand.source = ControlSource::AUTONOMY;
      outCommand.kind = CommandKind::MOVE;
      outCommand.driveMode = DriveMode::REVERSE;
      outCommand.durationMs = AUTONOMY_REVERSE_MS;
      _stateMs = now;
      _state = AutonomyState::BACKING_UP;
      return true;
    }
    case AutonomyState::BACKING_UP: {
      if (now - _stateMs >= AUTONOMY_REVERSE_MS) {
        outCommand.source = ControlSource::AUTONOMY;
        outCommand.kind = CommandKind::MOVE;
        outCommand.driveMode = DriveMode::TURN_RIGHT;
        outCommand.durationMs = AUTONOMY_TURN_MS;
        _stateMs = now;
        _state = AutonomyState::TURNING;
        return true;
      }
      break;
    }
    case AutonomyState::TURNING: {
      if (now - _stateMs >= AUTONOMY_TURN_MS) {
        outCommand.source = ControlSource::AUTONOMY;
        outCommand.kind = CommandKind::STOP;
        _stateMs = now;
        _state = AutonomyState::WAIT_FOR_CLEARANCE;
        return true;
      }
      break;
    }
    case AutonomyState::WAIT_FOR_CLEARANCE: {
      if (r.valid && r.distanceMm >= OBSTACLE_CLEAR_MM) {
        _state = AutonomyState::RESUMING;
      } else if (now - _stateMs >= AUTONOMY_CLEAR_TIMEOUT_MS) {
        _robot->clearRememberedDriveCommand();
        _state = AutonomyState::MONITORING;
      }
      break;
    }
    case AutonomyState::RESUMING: {
      DriveCommand saved;
      if (_robot->lastDriveCommand(saved) && saved.mode != DriveMode::STOPPED) {
        outCommand.source = ControlSource::AUTONOMY;
        outCommand.kind = CommandKind::MOVE;
        outCommand.driveMode = saved.mode;
        outCommand.durationMs = saved.durationMs;
      }
      _state = AutonomyState::MONITORING;
      return outCommand.kind != CommandKind::NONE;
    }
    default:
      _state = AutonomyState::MONITORING;
      break;
  }

  return false;
}
