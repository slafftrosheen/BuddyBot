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
  _hal->update();
  _actions->update();
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
  _actions->cancel();
  if (_hal->drive()) {
    _hal->drive()->disarm();
  }
}

void RobotAPI::stopAll() {
  _actions->cancel();
  if (_hal->drive()) {
    _hal->drive()->emergencyStop();
  }
}

void RobotAPI::move(DriveMode mode, uint16_t durationMs) {
  _actions->cancel();
  if (!_hal->drive()) return;

  _hal->drive()->drive(DriveCommand{mode, 0, 0, durationMs});
}

void RobotAPI::action(ActionId actionId) {
  _actions->start(actionId);
}

void RobotAPI::accessory(uint8_t index, bool active) {
  // Placeholder: wire to HAL accessory interfaces when configured.
  (void)index;
  (void)active;
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
