#pragma once
#pragma once
#include "Types.h"
#include "Persona.h"
#include "RobotHal.h"
#include "RobotActions.h"

class RobotAPI {
public:
  void begin(PersonaManager* persona, RobotHal* hal, RobotActions* actions);
  void update();

  Mood getMood() const;
  void setMood(Mood mood, bool playSound = true);
  void nextMood();
  void nextPersona();

  void armMotors();
  void disarmMotors();
  void stopAll();

  void move(DriveMode mode, uint16_t durationMs);
  void action(ActionId actionId);

  void accessory(uint8_t index, bool active);
  RangeReading rangeReading() const;
  bool obstacleDetected() const;

  const RobotBuildConfig& buildConfig() const;

  void setAutonomyEnabled(bool enabled);
  bool autonomyEnabled() const;

private:
  PersonaManager* _persona = nullptr;
  RobotHal* _hal = nullptr;
  RobotActions* _actions = nullptr;
  Mood _mood = Mood::IDLE;
  bool _autonomyEnabled = false;

  void playMoodSound();
  void playTone(uint16_t frequency, uint16_t durationMs);
};
