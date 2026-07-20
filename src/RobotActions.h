#pragma once

#include "Types.h"
#include "IDriveBase.h"
#include "IJoint.h"

class RobotHal;

enum class ActionDrivePolicy : uint8_t {
  PRESERVE_DRIVE,
  OWNS_DRIVE,
  STOP_DRIVE
};

class RobotActions {
public:
  void begin(RobotHal* hal);

  void update();

  bool isRunning() const;
  ActionId currentAction() const;

  void start(ActionId action);
  void cancel(bool stopDrive = false);

private:
  RobotHal* _hal = nullptr;

  ActionDrivePolicy drivePolicy(ActionId action) const;
  uint16_t timeoutFor(ActionId action) const;
  void finishAction(bool stopDrive);

  ActionId _action = ActionId::NONE;
  uint8_t _step = 0;

  uint32_t _stepStartedMs = 0;
  uint32_t _actionStartedMs = 0;

  bool stepElapsed(uint16_t durationMs) const;
  void nextStep();

  void updateWave();
  void updateLookLeft();
  void updateLookRight();
  void updateCelebrate();
  void updateDance();
  void updateGreet();
  void updateSleep();
};
