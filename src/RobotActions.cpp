#include "RobotActions.h"
#include "RobotHal.h"
#include "Config.h"

void RobotActions::begin(RobotHal* hal) {
  _hal = hal;
  _action = ActionId::NONE;
  _step = 0;
}

bool RobotActions::isRunning() const {
  return _action != ActionId::NONE;
}

ActionId RobotActions::currentAction() const {
  return _action;
}

void RobotActions::start(ActionId action) {
  if (_hal == nullptr) {
    return;
  }

  cancel();

  _action = action;
  _step = 0;
  _stepStartedMs = millis();
  _actionStartedMs = millis();
}

void RobotActions::cancel() {
  if (_hal != nullptr) {
    if (_hal->drive()) {
      _hal->drive()->emergencyStop();
    }
    if (_hal->leftArm()) _hal->leftArm()->rest();
    if (_hal->rightArm()) _hal->rightArm()->rest();
    if (_hal->head()) _hal->head()->rest();
  }

  _action = ActionId::NONE;
  _step = 0;
}

bool RobotActions::stepElapsed(uint16_t durationMs) const {
  return millis() - _stepStartedMs >= durationMs;
}

void RobotActions::nextStep() {
  _step++;
  _stepStartedMs = millis();
}

void RobotActions::update() {
  if (_action == ActionId::NONE || _hal == nullptr) {
    return;
  }

  if (millis() - _actionStartedMs > MAX_ACTION_MS) {
    cancel();
    return;
  }

  switch (_action) {
    case ActionId::WAVE:      updateWave(); break;
    case ActionId::LOOK_LEFT: updateLookLeft(); break;
    case ActionId::LOOK_RIGHT: updateLookRight(); break;
    case ActionId::CELEBRATE: updateCelebrate(); break;
    case ActionId::DANCE:     updateDance(); break;
    case ActionId::GREET:     updateGreet(); break;
    case ActionId::SLEEP:     updateSleep(); break;
    default: cancel(); break;
  }
}

void RobotActions::updateWave() {
  if (!_hal->leftArm()) return;

  switch (_step) {
    case 0:
      _hal->leftArm()->moveTo(ARM_LEFT_WAVE_IN_DEG);
      nextStep();
      break;

    case 1:
      if (stepElapsed(ACTION_STEP_MS)) {
        _hal->leftArm()->moveTo(ARM_LEFT_WAVE_OUT_DEG);
        nextStep();
      }
      break;

    case 2:
      if (stepElapsed(ACTION_STEP_MS)) {
        _hal->leftArm()->moveTo(ARM_LEFT_WAVE_IN_DEG);
        nextStep();
      }
      break;

    case 3:
      if (stepElapsed(ACTION_STEP_MS)) {
        _hal->leftArm()->moveTo(ARM_LEFT_WAVE_OUT_DEG);
        nextStep();
      }
      break;

    default:
      if (stepElapsed(ACTION_STEP_MS)) {
        cancel();
      }
      break;
  }
}

void RobotActions::updateLookLeft() {
  if (!_hal->head()) return;

  if (_step == 0) {
    _hal->head()->moveTo(HEAD_LEFT_DEG);
    nextStep();
    return;
  }

  if (_step == 1 && stepElapsed(650)) {
    _hal->head()->moveTo(HEAD_CENTER_DEG);
    nextStep();
    return;
  }

  if (_step == 2 && stepElapsed(280)) {
    _action = ActionId::NONE;
  }
}

void RobotActions::updateLookRight() {
  if (!_hal->head()) return;

  if (_step == 0) {
    _hal->head()->moveTo(HEAD_RIGHT_DEG);
    nextStep();
    return;
  }

  if (_step == 1 && stepElapsed(650)) {
    _hal->head()->moveTo(HEAD_CENTER_DEG);
    nextStep();
    return;
  }

  if (_step == 2 && stepElapsed(280)) {
    _action = ActionId::NONE;
  }
}

void RobotActions::updateCelebrate() {
  if (_step == 0) {
    if (_hal->leftArm()) _hal->leftArm()->moveTo(ARM_LEFT_UP_DEG);
    if (_hal->rightArm()) _hal->rightArm()->moveTo(ARM_RIGHT_UP_DEG);
    nextStep();
    return;
  }

  if (_step == 1) {
    if (!_hal->drive()) return;
    if (stepElapsed(350)) {
      if (_hal->drive()->isArmed()) {
        _hal->drive()->drive(DriveCommand{DriveMode::TURN_LEFT, 0, 0, 220});
      }
      nextStep();
    }
    return;
  }

  if (_step == 2) {
    if (!_hal->drive()) return;
    if (stepElapsed(280)) {
      if (_hal->drive()->isArmed()) {
        _hal->drive()->drive(DriveCommand{DriveMode::TURN_RIGHT, 0, 0, 440});
      }
      nextStep();
    }
    return;
  }

  if (_step == 3) {
    if (stepElapsed(500)) {
      cancel();
    }
  }
}

void RobotActions::updateDance() {
  if (_step == 0) {
    if (_hal->leftArm()) _hal->leftArm()->moveTo(ARM_LEFT_UP_DEG);
    if (_hal->rightArm()) _hal->rightArm()->moveTo(ARM_RIGHT_UP_DEG);
    if (_hal->drive() && _hal->drive()->isArmed()) {
      _hal->drive()->drive(DriveCommand{DriveMode::TURN_LEFT, 0, 0, 220});
    }
    nextStep();
    return;
  }

  if (_step == 1) {
    if (stepElapsed(280)) {
      if (_hal->leftArm()) _hal->leftArm()->moveTo(ARM_LEFT_REST_DEG);
      if (_hal->rightArm()) _hal->rightArm()->moveTo(ARM_RIGHT_REST_DEG);
      if (_hal->drive() && _hal->drive()->isArmed()) {
        _hal->drive()->drive(DriveCommand{DriveMode::TURN_RIGHT, 0, 0, 440});
      }
      nextStep();
    }
    return;
  }

  if (_step == 2) {
    if (stepElapsed(500)) {
      if (_hal->leftArm()) _hal->leftArm()->moveTo(ARM_LEFT_UP_DEG);
      if (_hal->rightArm()) _hal->rightArm()->moveTo(ARM_RIGHT_UP_DEG);
      if (_hal->drive() && _hal->drive()->isArmed()) {
        _hal->drive()->drive(DriveCommand{DriveMode::FORWARD, 0, 0, 250});
      }
      nextStep();
    }
    return;
  }

  if (_step == 3 && stepElapsed(320)) {
    cancel();
  }
}

void RobotActions::updateGreet() {
  if (!_hal->head()) return;

  if (_step == 0) {
    _hal->head()->moveTo(HEAD_LEFT_DEG);
    nextStep();
    return;
  }

  if (_step == 1 && stepElapsed(300)) {
    _action = ActionId::WAVE;
    _step = 0;
    _stepStartedMs = millis();
  }
}

void RobotActions::updateSleep() {
  if (_step == 0) {
    if (_hal->leftArm()) _hal->leftArm()->rest();
    if (_hal->rightArm()) _hal->rightArm()->rest();
    if (_hal->head()) _hal->head()->rest();
    nextStep();
    return;
  }

  if (_step == 1 && stepElapsed(400)) {
    _action = ActionId::NONE;
  }
}
