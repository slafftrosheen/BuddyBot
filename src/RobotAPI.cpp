#include "RobotAPI.h"
#include <M5Unified.h>
#include "BuildProfiles.h"

void RobotAPI::begin(
  PersonaManager* persona,
  RobotHal* hal,
  RobotActions* actions
) {
  _persona = persona;
  _hal = hal;
  _actions = actions;
  _mood = Mood::IDLE;
  _autonomyEnabled = false;
  
  if (_actions) {
    _actions->begin(hal, persona, this);
  }
  
  _diagnostics.begin(hal);
  _safety.begin();
  _expressions.begin();
}

void RobotAPI::update() {
  if (_hal) _hal->update();
  if (_actions) _actions->update();
  _safety.update(rangeReading(), millis());
  
  if (obstacleDetected()) {
    _expressions.notifyObstacle();
  }

  _expressions.update(millis(), *_persona);
  _diagnostics.update(millis());
}

Mood RobotAPI::getMood() const {
  return _mood;
}

Mood RobotAPI::baseMood() const {
  return _expressions.baseMood();
}

void RobotAPI::playTone(uint16_t frequency, uint16_t durationMs) {
  float pitch = _persona->current().voicePitch;
  M5.Speaker.tone(uint16_t(frequency * pitch), durationMs);
}

void RobotAPI::playMoodSound() {
  switch (_mood) {
    case Mood::HAPPY:
      playTone(523, 50);
      playTone(659, 50);
      playTone(784, 70);
      break;

    case Mood::CURIOUS:
      playTone(440, 85);
      playTone(554, 100);
      break;

    case Mood::SLEEPY:
      playTone(440, 90);
      playTone(370, 90);
      playTone(330, 110);
      break;

    case Mood::EXCITED:
      playTone(523, 40);
      playTone(659, 40);
      playTone(784, 40);
      playTone(1047, 60);
      break;

    case Mood::ALERT:
      playTone(220, 140);
      playTone(220, 140);
      break;

    default:
      playTone(523, 40);
      break;
  }
}

void RobotAPI::setMood(Mood mood, bool playSound) {
  _mood = mood;
  _expressions.setBaseMood(mood);

  if (_mood == Mood::ALERT) {
    stopAll();
    _expressions.play(ExpressionId::SCARED);
  } else if (_mood == Mood::HAPPY) {
    _expressions.play(ExpressionId::GIGGLE, 1000);
  } else if (_mood == Mood::CURIOUS) {
    _expressions.play(ExpressionId::THINK, 1200);
  } else if (_mood == Mood::SLEEPY) {
    _actions->start(ActionId::SLEEP);
    _expressions.play(ExpressionId::SLEEP_YAWN);
  }

  if (playSound) {
    playMoodSound();
  }
}

void RobotAPI::nextMood() {
  uint8_t next = (uint8_t(_mood) + 1) % uint8_t(Mood::COUNT);
  setMood(Mood(next));
}

void RobotAPI::nextPersona() {
  _persona->next();
  playMoodSound();
}

const char* RobotAPI::personaName() const {
  if (!_persona) return "unknown";
  return _persona->current().name;
}

void RobotAPI::armMotors() {
  if (!ALLOW_MOTOR_ARMING) {
    _expressions.play(ExpressionId::WORRIED, 2000);
    return;
  }
  if (_hal->drive()) {
    if (!_hal->drive()->isArmed()) {
       _expressions.play(ExpressionId::PROUD, 1500);
    }
    _hal->drive()->arm();
  }
}

void RobotAPI::disarmMotors() {
  clearRememberedDriveCommand();
  _actions->cancel(true);
  if (_hal && _hal->drive()) {
    _hal->drive()->disarm();
  }
  _safety.recordExternalStop(SafetyStopReason::DISARMED, millis());
  
  if (RETURN_MANIPULATORS_ON_SAFETY_STOP && _hal) {
    if (_hal->leftArm()) _hal->leftArm()->rest();
    if (_hal->rightArm()) _hal->rightArm()->rest();
    if (_hal->head()) _hal->head()->rest();
  }
  _expressions.clear();
}

void RobotAPI::stopAll() {
  clearRememberedDriveCommand();
  _actions->cancel(true);
  if (_hal && _hal->drive()) {
    _hal->drive()->emergencyStop();
  }
  
  // Record MANUAL_STOP only if we didn't already have an obstacle stop active
  if (!_safety.blocksForwardMotion()) {
    _safety.recordExternalStop(SafetyStopReason::MANUAL_STOP, millis());
  }

  if (RETURN_MANIPULATORS_ON_SAFETY_STOP && _hal) {
    if (_hal->leftArm()) _hal->leftArm()->rest();
    if (_hal->rightArm()) _hal->rightArm()->rest();
    if (_hal->head()) _hal->head()->rest();
  }
}



bool RobotAPI::isArmed() const {
  if (_hal && _hal->drive()) {
    return _hal->drive()->isArmed();
  }
  return false;
}

ActionId RobotAPI::currentAction() const {
  if (_actions) {
    return _actions->currentAction();
  }
  return ActionId::NONE;
}

bool RobotAPI::driveAvailable() const {
  return _hal && _hal->drive() && _hal->drive()->isConnected();
}

bool RobotAPI::fourWheelDriveConfigured() const {
  if (!_hal || !_hal->drive()) return false;
  if (_hal->buildConfig().driveType == DriveControllerType::SERVO8_CONTINUOUS) {
    const ServoChannelConfig* cfg = getActiveServoConfig();
    if (cfg) {
      for (int i = 0; i < 8; i++) {
        if (cfg[i].enabled && cfg[i].role == ServoRole::DRIVE_REAR_LEFT) {
          return true;
        }
      }
    }
  }
  return false;
}

ActuatorCapabilities RobotAPI::actuatorCapabilities() const {
  if (_hal) return _hal->capabilities();
  return {};
}

ManipulatorState RobotAPI::manipulatorState(ManipulatorId id) const {
  if (_hal) return _hal->manipulatorState(id);
  return {};
}

bool RobotAPI::manipulatorAvailable(ServoRole role) const {
  if (!_hal) return false;
  switch (role) {
    case ServoRole::HEAD: return _hal->head() != nullptr;
    case ServoRole::LEFT_ARM: return _hal->leftArm() != nullptr;
    case ServoRole::RIGHT_ARM: return _hal->rightArm() != nullptr;
    case ServoRole::ACCESSORY_1: return _hal->accessory(1) != nullptr;
    default: return false;
  }
}

void RobotAPI::moveJointTo(ServoRole role, int16_t angle, uint16_t durationMs) {
  if (!_hal) return;
  IJoint* joint = nullptr;
  switch (role) {
    case ServoRole::HEAD: joint = _hal->head(); break;
    case ServoRole::LEFT_ARM: joint = _hal->leftArm(); break;
    case ServoRole::RIGHT_ARM: joint = _hal->rightArm(); break;
    case ServoRole::ACCESSORY_1: joint = _hal->accessory(1); break;
    default: break;
  }
  if (joint) {
    joint->moveTo(angle, durationMs);
  }
}

void RobotAPI::restJoint(ServoRole role) {
  if (!_hal) return;
  IJoint* joint = nullptr;
  switch (role) {
    case ServoRole::HEAD: joint = _hal->head(); break;
    case ServoRole::LEFT_ARM: joint = _hal->leftArm(); break;
    case ServoRole::RIGHT_ARM: joint = _hal->rightArm(); break;
    case ServoRole::ACCESSORY_1: joint = _hal->accessory(1); break;
    default: break;
  }
  if (joint) {
    joint->rest();
  }
}

bool RobotAPI::move(DriveMode mode, uint16_t durationMs, bool cancelAction) {
  if (cancelAction && _actions) _actions->cancel(false);
  if (!_hal || !_hal->drive()) return false;
  if (!isArmed()) return false;

  if (!_safety.allowsDrive(mode)) {
    // Safety blocks this move
    if (_hal->drive()) _hal->drive()->emergencyStop();
    
    SafetyStopReason reason = SafetyStopReason::OBSTACLE_BLOCKED;
    if (_safety.status().state == ObstacleSafetyState::SENSOR_UNAVAILABLE) {
      if (!_safety.status().rangeValid) reason = SafetyStopReason::RANGE_SENSOR_INVALID;
      // Stale logic could also be added, but invalid/unavailable is enough for now
    }
    
    _safety.recordExternalStop(reason, millis());
    return false;
  }
  
  _safety.clearExternalStopIfSafe();

  DriveCommand cmd = {mode, 0, 0, durationMs};
  if (mode != DriveMode::STOPPED) {
    rememberDriveCommand(cmd);
  } else {
    clearRememberedDriveCommand();
  }

  _hal->drive()->drive(cmd);
  return true;
}

void RobotAPI::action(ActionId actionId) {
  _actions->start(actionId);
  
  if (actionId == ActionId::WAVE) {
    _expressions.play(ExpressionId::GIGGLE, 2000);
  } else if (actionId == ActionId::CELEBRATE) {
    _expressions.play(ExpressionId::PROUD, 3000);
  } else if (actionId == ActionId::SLEEP) {
    _expressions.play(ExpressionId::SLEEP_YAWN);
  }
}

void RobotAPI::playExpression(ExpressionId expression, uint16_t durationMs) {
  _expressions.play(expression, durationMs);
}

void RobotAPI::setAttention(AttentionTarget target) {
  _expressions.setAttention(target);
}

ExpressionId RobotAPI::expression() const {
  return _expressions.current();
}

const ExpressionEngine& RobotAPI::expressionEngine() const {
  return _expressions;
}

void RobotAPI::setAccessoryPosition(uint8_t index, bool active) {
  if (!_hal) return;
  IJoint* acc = _hal->accessory(index);
  if (!acc) {
    Serial.printf("ERR: Accessory %u not configured or invalid\n", index);
    return;
  }
  if (active) {
    acc->moveTo(ACCESSORY_ACTIVE_DEG);
  } else {
    acc->rest();
  }
}

RangeReading RobotAPI::rangeReading() const {
  if (!_hal->range()) return {};
  return _hal->range()->reading();
}

bool RobotAPI::obstacleDetected() const {
  return _safety.blocksForwardMotion();
}

const RobotBuildConfig& RobotAPI::buildConfig() const {
  return _hal->buildConfig();
}

void RobotAPI::setAutonomyEnabled(bool enabled) {
  _autonomyEnabled = enabled;
}

bool RobotAPI::autonomyEnabled() const {
  return _autonomyEnabled;
}

void RobotAPI::rememberDriveCommand(const DriveCommand& cmd) {
  _lastManualDriveCmd = cmd;
  _lastManualDriveCmdMs = millis();
  _hasSavedCmd = true;
}

bool RobotAPI::lastDriveCommand(DriveCommand& out) const {
  if (!_hasSavedCmd) return false;
  out = _lastManualDriveCmd;
  return true;
}

void RobotAPI::clearRememberedDriveCommand() {
  _hasSavedCmd = false;
  _lastManualDriveCmd = {DriveMode::STOPPED, 0, 0, 0};
  _lastManualDriveCmdMs = millis();
}

const ObstacleSafetyStatus& RobotAPI::obstacleSafetyStatus() const {
  static ObstacleSafetyStatus _statusCache;
  _statusCache = _safety.status();
  return _statusCache;
}

bool RobotAPI::forwardMotionAllowed() const {
  return !_safety.blocksForwardMotion();
}

SafetyStopReason RobotAPI::lastSafetyStopReason() const {
  return _safety.status().lastStopReason;
}

RangeSensorHealth RobotAPI::rangeSensorHealth() const {
  if (_hal && _hal->range()) return _hal->range()->health();
  return RangeSensorHealth::UNINITIALIZED;
}

uint16_t RobotAPI::rangeConsecutiveInvalid() const {
  if (_hal && _hal->range()) return _hal->range()->consecutiveInvalidSamples();
  return 0;
}


ServoDiagnostics* RobotAPI::diagnostics() {
  return &_diagnostics;
}
