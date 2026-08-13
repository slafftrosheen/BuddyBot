#include "RobotAPI.h"
#include <M5Unified.h>
#include "BuildProfiles.h"
#include "SafetySupervisor.h"
#include "BootDiagnostics.h"

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
  _imu.begin();
  _expressions.begin();
}

void RobotAPI::setSafetySupervisor(SafetySupervisor* supervisor) {
  _safetySupervisor = supervisor;
}

void RobotAPI::setBootDiagnostics(const BootDiagnostics* diag) {
  _bootDiag = diag;
}

RuntimeSnapshot RobotAPI::runtimeSnapshot() const {
  RuntimeSnapshot snap;
  snap.capturedAtMs = millis();

  // Safety
  snap.safety = buildSafetySnapshot(_safetySupervisor, millis());

  // Drive
  snap.drive.available = driveAvailable();
  snap.drive.active = false;
  snap.drive.forward = false;
  snap.drive.blocked = obstacleDetected();
  snap.drive.mode = static_cast<uint8_t>(DriveMode::STOPPED);
  snap.drive.lastCommandAtMs = lastDriveCommandAtMs();
  if (_hal && _hal->drive()) {
    DriveMode m = _hal->drive()->driveMode();
    snap.drive.active = (m != DriveMode::STOPPED);
    snap.drive.forward = (m == DriveMode::FORWARD);
    snap.drive.mode = static_cast<uint8_t>(m);
  }

  // Range
  RangeReading range = rangeReading();
  snap.range.available = _hal && _hal->range() != nullptr;
  snap.range.valid = range.valid;
  snap.range.distanceMm = range.distanceMm;
  snap.range.health = static_cast<uint8_t>(rangeSensorHealth());
  snap.range.sampleTimeMs = range.sampleTimeMs;
  snap.range.consecutiveInvalid = rangeConsecutiveInvalid();

  // IMU
  const ImuReading& imu = imuReading();
  snap.imu.available = imu.available;
  snap.imu.valid = imu.valid;
  snap.imu.sampleTimeMs = imu.sampleTimeMs;
  snap.imu.accelXG = imu.accelXG;
  snap.imu.accelYG = imu.accelYG;
  snap.imu.accelZG = imu.accelZG;
  snap.imu.gyroXDps = imu.gyroXDps;
  snap.imu.gyroYDps = imu.gyroYDps;
  snap.imu.gyroZDps = imu.gyroZDps;

  // Behavior
  snap.behavior.actionRunning = _actions ? _actions->isRunning() : false;
  snap.behavior.action = static_cast<uint8_t>(currentAction());
  snap.behavior.mood = static_cast<uint8_t>(getMood());
  snap.behavior.persona = static_cast<uint8_t>(_persona ? _persona->id() : PersonaId::PIXEL);
  snap.behavior.autonomyEnabled = autonomyEnabled();

  // Hardware
  snap.hardware.driveAvailable = snap.drive.available;
  snap.hardware.rangeAvailable = snap.range.available;
  snap.hardware.imuAvailable = snap.imu.available;
  snap.hardware.servoBusPresent = _bootDiag ? _bootDiag->status().servoBusPresent : false;
  snap.hardware.sonicPresent = _bootDiag ? _bootDiag->status().sonicPresent : false;

  return snap;
}

void RobotAPI::update() {
  if (_hal) _hal->update();
  if (_actions) _actions->update();
  const uint32_t nowMs = millis();
  _safety.update(rangeReading(), nowMs);
  _imu.update(nowMs);

  if (_hal && _hal->drive() &&
      _hal->drive()->driveMode() == DriveMode::FORWARD &&
      _safety.blocksForwardMotion()) {
    _hal->drive()->emergencyStop();
    clearRememberedDriveCommand();
    _safety.recordExternalStop(
      _safety.status().state == ObstacleSafetyState::SENSOR_UNAVAILABLE
        ? SafetyStopReason::RANGE_SENSOR_INVALID
        : SafetyStopReason::OBSTACLE_BLOCKED,
      nowMs
    );
    const RangeSensorHealth sensorHealth = rangeSensorHealth();
    if (_safetySupervisor &&
        _safety.status().state == ObstacleSafetyState::SENSOR_UNAVAILABLE &&
        (sensorHealth == RangeSensorHealth::INVALID ||
         sensorHealth == RangeSensorHealth::UNAVAILABLE)) {
      _safetySupervisor->emergencyStop(SafetyFault::RANGE_SENSOR_INVALID, nowMs);
    }
  }
  
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
    if (_actions && (!_safetySupervisor || _safetySupervisor->mayMoveManipulators())) {
      _actions->start(ActionId::SLEEP);
    }
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

bool RobotAPI::armMotors() {
  if (!ALLOW_MOTOR_ARMING) {
    _expressions.play(ExpressionId::WORRIED, 2000);
    return false;
  }
  if (!_safetySupervisor || !_safetySupervisor->mayEnableDrive()) {
    _expressions.play(ExpressionId::WORRIED, 2000);
    return false;
  }

  if (_hal && _hal->drive()) {
    if (!_hal->drive()->isArmed()) {
       _expressions.play(ExpressionId::PROUD, 1500);
    }
    _hal->drive()->arm();
    if (_hal->drive()->isArmed()) {
      return true;
    }
  }

  _safetySupervisor->notifyArmFailed(millis());
  return false;
}

void RobotAPI::disarmMotors() {
  clearRememberedDriveCommand();
  const bool returnManipulatorsToRest =
    !_safetySupervisor || _safetySupervisor->mayMoveManipulators();
  _actions->cancel(true, returnManipulatorsToRest);
  if (_hal && _hal->drive()) {
    _hal->drive()->disarm();
  }
  _safety.recordExternalStop(SafetyStopReason::DISARMED, millis());
  
  if (RETURN_MANIPULATORS_ON_SAFETY_STOP && returnManipulatorsToRest && _hal) {
    if (_hal->leftArm()) _hal->leftArm()->rest();
    if (_hal->rightArm()) _hal->rightArm()->rest();
    if (_hal->head()) _hal->head()->rest();
  }
  _expressions.clear();
}

void RobotAPI::stopAll() {
  clearRememberedDriveCommand();
  const bool returnManipulatorsToRest =
    !_safetySupervisor || _safetySupervisor->mayMoveManipulators();
  _actions->cancel(true, returnManipulatorsToRest);
  if (_hal && _hal->drive()) {
    _hal->drive()->emergencyStop();
  }
  
  // Record MANUAL_STOP only if we didn't already have an obstacle stop active
  if (!_safety.blocksForwardMotion()) {
    _safety.recordExternalStop(SafetyStopReason::MANUAL_STOP, millis());
  }

  if (RETURN_MANIPULATORS_ON_SAFETY_STOP && returnManipulatorsToRest && _hal) {
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
  if (!_hal || (_safetySupervisor && !_safetySupervisor->mayMoveManipulators())) return;
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
  if (!_hal || (_safetySupervisor && !_safetySupervisor->mayMoveManipulators())) return;
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

bool RobotAPI::move(DriveMode mode, uint16_t durationMs, bool cancelAction, ControlSource source) {
  if (cancelAction && _actions) _actions->cancel(false);
  if (!_hal || !_hal->drive()) return false;
  if (!isArmed()) return false;

  if (mode == DriveMode::STOPPED) {
    stopAll();
    return true;
  }

  if (!_safetySupervisor ||
      !_safetySupervisor->requestDrive(
        mode == DriveMode::FORWARD,
        source == ControlSource::AUTONOMY,
        millis()
      )) {
    if (_hal && _hal->drive()) {
      _hal->drive()->emergencyStop();
    }
    return false;
  }

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
  if (_safetySupervisor && !_safetySupervisor->mayMoveManipulators()) {
    return;
  }
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
  if (!_hal || (_safetySupervisor && !_safetySupervisor->mayMoveManipulators())) return;
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

bool RobotAPI::autonomyMotionAllowed() const {
  return _safetySupervisor &&
         _safetySupervisor->autonomyEnabled() &&
         _safetySupervisor->autonomyMotionAllowed(millis());
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

uint32_t RobotAPI::lastDriveCommandAtMs() const {
  return _hasSavedCmd ? _lastManualDriveCmdMs : 0;
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

void RobotAPI::recordSafetyStop(SafetyStopReason reason) {
  _safety.recordExternalStop(reason, millis());
}

RangeSensorHealth RobotAPI::rangeSensorHealth() const {
  if (_hal && _hal->range()) return _hal->range()->health();
  return RangeSensorHealth::UNINITIALIZED;
}

uint16_t RobotAPI::rangeConsecutiveInvalid() const {
  if (_hal && _hal->range()) return _hal->range()->consecutiveInvalidSamples();
  return 0;
}

const ImuReading& RobotAPI::imuReading() const {
  return _imu.reading();
}

SafetyState RobotAPI::safetyState() const {
  return _safetySupervisor ? _safetySupervisor->state() : SafetyState::BOOT;
}

SafetyFault RobotAPI::safetyFault() const {
  return _safetySupervisor ? _safetySupervisor->fault() : SafetyFault::BOOT_INCOMPLETE;
}

uint32_t RobotAPI::safetyStateChangedAtMs() const {
  return _safetySupervisor ? _safetySupervisor->stateChangedAtMs() : 0;
}


ServoDiagnostics* RobotAPI::diagnostics() {
  return &_diagnostics;
}
