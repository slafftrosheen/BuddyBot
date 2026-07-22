#include "ControlRouter.h"
#include "BuildProfiles.h"
#include "WifiControl.h"

void ControlRouter::begin(RobotAPI* robot, SystemStatus* status) {
  _robot = robot;
  _status = status;
}

bool ControlRouter::execute(const RobotCommand& cmd) {
  if (_robot == nullptr) return false;

  bool isTelemetryQuery = (
    cmd.kind == CommandKind::RANGE_QUERY ||
    cmd.kind == CommandKind::PROFILE_LIST ||
    cmd.kind == CommandKind::PROFILE_SHOW ||
    cmd.kind == CommandKind::STATUS_QUERY ||
    cmd.kind == CommandKind::CTRL_WIFI_STATUS
  );

  if (!isTelemetryQuery) {
    _currentEpoch++;
  }

  bool isIntervention = false;
  if (cmd.kind == CommandKind::STOP || cmd.kind == CommandKind::DISARM) {
    isIntervention = true;
  } else if (!isTelemetryQuery && cmd.source != ControlSource::WIFI) {
    // Any non-WIFI active command supersedes queued WIFI commands
    isIntervention = true;
  }

  if (isIntervention) {
    _lastInterventionEpoch = _currentEpoch;
  }

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
      return _robot->move(cmd.driveMode, cmd.durationMs);

    case CommandKind::ACTION:
      _robot->action(cmd.action);
      return true;

    case CommandKind::PLAY_EXPRESSION:
      _robot->playExpression(cmd.expression, cmd.durationMs);
      return true;

    case CommandKind::SET_ATTENTION:
      _robot->setAttention(cmd.attention);
      return true;

    case CommandKind::ACCESSORY:
      if (cmd.index > 0 && cmd.index <= 3) {
        _robot->setAccessoryPosition(cmd.index, cmd.flag);
      }
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

    case CommandKind::VERSION: {
      if (_status) {
        FirmwareIdentity id = getFirmwareIdentity();
        Serial.printf("name=%s\n", id.name);
        Serial.printf("version=%s\n", id.version);
        Serial.printf("channel=%s\n", id.channel);
        Serial.printf("profile=%s\n", id.buildProfile);
        Serial.printf("compile_date=%s\n", id.compileDate);
        Serial.printf("compile_time=%s\n", id.compileTime);
      }
      return true;
    }

    case CommandKind::DIAG_BOOT: {
      if (_status) {
        // Output from SystemStatus which has access to boot diag
        _status->printStatus(); // the bool flag and phase are printed here now
      }
      return true;
    }

    case CommandKind::PRINT_EVENTS: {
      if (_status) {
        _status->printEvents();
      }
      return true;
    }

    case CommandKind::CTRL_SENSOR_STATUS: {
      RangeSensorHealth health = _robot->rangeSensorHealth();
      RangeReading r = _robot->rangeReading();
      const char* hStr = "unknown";
      switch(health) {
        case RangeSensorHealth::UNINITIALIZED: hStr = "uninitialized"; break;
        case RangeSensorHealth::READY: hStr = "ready"; break;
        case RangeSensorHealth::STALE: hStr = "stale"; break;
        case RangeSensorHealth::INVALID: hStr = "invalid"; break;
        case RangeSensorHealth::UNAVAILABLE: hStr = "unavailable"; break;
      }
      Serial.printf("sonic=%s\n", (health != RangeSensorHealth::UNAVAILABLE) ? "connected" : "unavailable");
      Serial.printf("health=%s\n", hStr);
      if (r.valid) {
        Serial.printf("range=%umm\n", r.distanceMm);
      } else {
        Serial.println("range=invalid");
      }
      Serial.printf("stale=%s\n", (health == RangeSensorHealth::STALE) ? "true" : "false");
      Serial.printf("consecutive_invalid=%u\n", _robot->rangeConsecutiveInvalid());
      return true;
    }

    case CommandKind::CTRL_SAFETY_STATUS: {
      const ObstacleSafetyStatus& st = _robot->obstacleSafetyStatus();
      const char* sStr = "unknown";
      switch(st.state) {
        case ObstacleSafetyState::CLEAR: sStr = "CLEAR"; break;
        case ObstacleSafetyState::CAUTION: sStr = "CAUTION"; break;
        case ObstacleSafetyState::BLOCKED: sStr = "BLOCKED"; break;
        case ObstacleSafetyState::SENSOR_UNAVAILABLE: sStr = "SENSOR_UNAVAILABLE"; break;
      }
      
      const char* rStr = "unknown";
      switch(st.lastStopReason) {
        case SafetyStopReason::NONE: rStr = "NONE"; break;
        case SafetyStopReason::OBSTACLE_BLOCKED: rStr = "OBSTACLE_BLOCKED"; break;
        case SafetyStopReason::RANGE_SENSOR_STALE: rStr = "RANGE_SENSOR_STALE"; break;
        case SafetyStopReason::RANGE_SENSOR_INVALID: rStr = "RANGE_SENSOR_INVALID"; break;
        case SafetyStopReason::AUTONOMY_TIMEOUT: rStr = "AUTONOMY_TIMEOUT"; break;
        case SafetyStopReason::MANUAL_STOP: rStr = "MANUAL_STOP"; break;
        case SafetyStopReason::CONTROLLER_DISCONNECT: rStr = "CONTROLLER_DISCONNECT"; break;
        case SafetyStopReason::CONTROLLER_LEASE_EXPIRED: rStr = "CONTROLLER_LEASE_EXPIRED"; break;
        case SafetyStopReason::DRIVE_WATCHDOG: rStr = "DRIVE_WATCHDOG"; break;
        case SafetyStopReason::DISARMED: rStr = "DISARMED"; break;
      }
      
      Serial.printf("safety_state=%s\n", sStr);
      Serial.printf("forward_blocked=%s\n", st.forwardMotionBlocked ? "true" : "false");
      Serial.printf("last_stop_reason=%s\n", rStr);
      Serial.printf("last_stop_time=%u\n", st.lastStopMs);
      return true;
    }

    case CommandKind::CTRL_AUTONOMY_STATUS: {
      Serial.printf("autonomy_mode=%s\n", _robot->autonomyEnabled() ? "ASSISTED_AVOIDANCE" : "OFF");
      // AutonomyManager state is separate, we'd need to expose it through RobotAPI or just print this
      Serial.printf("autonomy_enabled=%s\n", _robot->autonomyEnabled() ? "true" : "false");
      const ObstacleSafetyStatus& st = _robot->obstacleSafetyStatus();
      bool ready = _robot->isArmed() && st.state != ObstacleSafetyState::SENSOR_UNAVAILABLE;
      Serial.printf("prerequisites_met=%s\n", ready ? "true" : "false");
      return true;
    }

    case CommandKind::AUTONOMY_SET:
      _robot->setAutonomyEnabled(cmd.flag);
      Serial.printf("autonomy=%s\n", cmd.flag ? "ON" : "OFF");
      return true;

    case CommandKind::CTRL_WIFI_STATUS:
      if (_wifi) {
        Serial.printf("wifi_ssid=%s\n", _wifi->apSsid());
        Serial.printf("wifi_ip=%s\n", _wifi->apIp());
        Serial.printf("wifi_running=%s\n", _wifi->running() ? "true" : "false");
        Serial.printf("wifi_clients=%u\n", _wifi->clientCount());
        Serial.printf("wifi_controller=%s\n", _wifi->controllerPresent() ? "true" : "false");
        Serial.printf("wifi_pairing_available=%s\n", _wifi->pairingAvailable() ? "true" : "false");
      }
      return true;

    case CommandKind::CTRL_WIFI_ON:
      if (_wifi) _wifi->start();
      return true;

    case CommandKind::CTRL_WIFI_OFF:
      if (_wifi) _wifi->stop();
      return true;

    case CommandKind::CTRL_WIFI_PAIR:
      if (_wifi) {
        if (_wifi->controllerPresent()) {
          Serial.println("ERR controller active");
        } else {
          _wifi->requestNewPairingCode();
        }
      }
      return true;

    case CommandKind::SERVO_TEST:
      if (cmd.source != ControlSource::SERIAL_CTRL) return false;
      if (cmd.arg1 == -1) {
        _robot->diagnostics()->unlock();
        Serial.println("SERVO TEST UNLOCKED");
      } else if (cmd.arg1 == -2) {
        _robot->diagnostics()->stopTest();
        Serial.println("SERVO TEST STOPPED");
      } else {
        if (_robot->diagnostics()->testWheel((ServoRole)cmd.arg1, cmd.arg2)) {
          Serial.printf("TEST WHEEL %d SPEED %d\n", cmd.arg1, cmd.arg2);
        } else {
          Serial.println("TEST REJECTED (LOCKED OR INVALID)");
        }
      }
      return true;

    case CommandKind::JOINT_MOVE:
      // Can be used via WiFi but typically useful for testing
      _robot->moveJointTo((ServoRole)cmd.arg1, cmd.arg2, cmd.durationMs);
      return true;

    case CommandKind::JOINT_REST:
      _robot->restJoint((ServoRole)cmd.arg1);
      return true;

    default:
      return false;
  }
}
