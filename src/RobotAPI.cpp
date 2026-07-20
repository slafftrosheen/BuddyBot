#include "RobotAPI.h"
#include <M5Unified.h>

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
}

void RobotAPI::update() {
  if (_hal) _hal->update();
  if (_actions) _actions->update();
}

Mood RobotAPI::getMood() const {
  return _mood;
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

  if (_mood == Mood::ALERT) {
    stopAll();
  }

  if (_mood == Mood::SLEEPY) {
    _actions->start(ActionId::SLEEP);
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

void RobotAPI::armMotors() {
  if (_hal->drive()) {
    _hal->drive()->arm();
  }
}

void RobotAPI::disarmMotors() {
  clearRememberedDriveCommand();
  _actions->cancel(true);
  if (_hal && _hal->drive()) {
    _hal->drive()->disarm();
  }
}

void RobotAPI::stopAll() {
  clearRememberedDriveCommand();
  _actions->cancel(true);
  if (_hal && _hal->drive()) {
    _hal->drive()->emergencyStop();
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

void RobotAPI::move(DriveMode mode, uint16_t durationMs) {
  _actions->cancel(false);
  if (!_hal || !_hal->drive()) return;

  DriveCommand cmd = {mode, 0, 0, durationMs};
  if (mode != DriveMode::STOPPED) {
    rememberDriveCommand(cmd);
  } else {
    clearRememberedDriveCommand();
  }

  _hal->drive()->drive(cmd);
}

void RobotAPI::action(ActionId actionId) {
  _actions->start(actionId);
}

void RobotAPI::accessory(uint8_t index, bool active) {
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
  if (!buildConfig().enableObstacleStop) return false;
  if (!_hal->range()) return false;
  RangeReading r = _hal->range()->reading();
  return r.valid && r.distanceMm > 0 && r.distanceMm < OBSTACLE_STOP_MM;
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
