#include "RobotActions.h"
#include "RobotHal.h"
#include "RobotAPI.h"
#include "Persona.h"
#include "ServoJoint.h"

void RobotActions::begin(RobotHal* hal, PersonaManager* personas, RobotAPI* robot) {
  _hal = hal;
  _personas = personas;
  _robot = robot;
  _action = ActionId::NONE;
  _step = 0;
}

bool RobotActions::isRunning() const {
  return _action != ActionId::NONE;
}

ActionId RobotActions::currentAction() const {
  return _action;
}

ActionDrivePolicy RobotActions::drivePolicy(ActionId action) const {
  switch (action) {
    case ActionId::WAVE:
    case ActionId::LOOK_LEFT:
    case ActionId::LOOK_RIGHT:
    case ActionId::GREET:
      return ActionDrivePolicy::PRESERVE_DRIVE;
    case ActionId::CELEBRATE:
    case ActionId::DANCE:
      return ActionDrivePolicy::OWNS_DRIVE;
    case ActionId::SLEEP:
      return ActionDrivePolicy::STOP_DRIVE;
    default:
      return ActionDrivePolicy::PRESERVE_DRIVE;
  }
}

uint16_t RobotActions::timeoutFor(ActionId action) const {
  if (action >= ActionId::COUNT) return 0;
  return ACTION_TIMEOUT_MS[uint8_t(action)];
}

void RobotActions::start(ActionId action) {
  if (_hal == nullptr) {
    return;
  }

  ActionDrivePolicy prevPolicy = drivePolicy(_action);
  ActionDrivePolicy newPolicy = drivePolicy(action);

  bool stopDrive = (prevPolicy == ActionDrivePolicy::OWNS_DRIVE) || (newPolicy == ActionDrivePolicy::STOP_DRIVE);
  cancel(stopDrive);

  _action = action;
  _step = 0;
  _stepStartedMs = millis();
  _actionStartedMs = millis();
}

void RobotActions::cancel(bool stopDrive) {
  if (_hal != nullptr) {
    if (stopDrive && _hal->drive()) {
      _hal->drive()->emergencyStop();
    }
    if (_hal->leftArm()) _hal->leftArm()->rest();
    if (_hal->rightArm()) _hal->rightArm()->rest();
    if (_hal->head()) _hal->head()->rest();
  }

  _action = ActionId::NONE;
  _step = 0;
}

void RobotActions::finishAction(bool stopDrive) {
  if (_hal != nullptr) {
    if (stopDrive && _hal->drive()) {
      _hal->drive()->emergencyStop();
    }
    switch (_action) {
      case ActionId::WAVE:
        if (_hal->leftArm()) _hal->leftArm()->rest();
        break;
      case ActionId::LOOK_LEFT:
      case ActionId::LOOK_RIGHT:
        if (_hal->head()) _hal->head()->rest();
        break;
      case ActionId::CELEBRATE:
      case ActionId::DANCE:
        if (_hal->leftArm()) _hal->leftArm()->rest();
        if (_hal->rightArm()) _hal->rightArm()->rest();
        break;
      case ActionId::GREET:
        if (_hal->head()) _hal->head()->rest();
        if (_hal->leftArm()) _hal->leftArm()->rest();
        break;
      case ActionId::SLEEP:
        if (_hal->leftArm()) _hal->leftArm()->rest();
        if (_hal->rightArm()) _hal->rightArm()->rest();
        if (_hal->head()) _hal->head()->rest();
        break;
      default:
        break;
    }
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

  uint16_t timeout = timeoutFor(_action);
  if (timeout > 0 && millis() - _actionStartedMs > timeout) {
    cancel(drivePolicy(_action) == ActionDrivePolicy::OWNS_DRIVE || drivePolicy(_action) == ActionDrivePolicy::STOP_DRIVE);
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
  if (!_hal->leftArm() || !_personas) {
    finishAction(false);
    return;
  }

  const PersonaGestureProfile& gestures = _personas->current().gestures;
  ServoJoint* leftArm = static_cast<ServoJoint*>(_hal->leftArm());
  if (!leftArm) return; // shouldn't happen but safe

  switch (_step) {
    case 0:
      leftArm->moveTo(90 + gestures.waveAmplitudeDeg, gestures.waveStepMs, JointEasing::EASE_IN_OUT);
      nextStep();
      break;

    case 1:
      if (stepElapsed(gestures.waveStepMs)) {
        leftArm->moveTo(90 - gestures.waveAmplitudeDeg, gestures.waveStepMs, JointEasing::EASE_IN_OUT);
        nextStep();
      }
      break;

    case 2:
      if (stepElapsed(gestures.waveStepMs)) {
        leftArm->moveTo(90 + gestures.waveAmplitudeDeg, gestures.waveStepMs, JointEasing::EASE_IN_OUT);
        nextStep();
      }
      break;

    case 3:
      if (stepElapsed(gestures.waveStepMs)) {
        leftArm->moveTo(90 - gestures.waveAmplitudeDeg, gestures.waveStepMs, JointEasing::EASE_IN_OUT);
        nextStep();
      }
      break;

    default:
      if (stepElapsed(gestures.waveStepMs)) {
        finishAction(false);
      }
      break;
  }
}

void RobotActions::updateLookLeft() {
  if (!_hal->head() || !_personas) {
    finishAction(false);
    return;
  }
  
  const PersonaGestureProfile& gestures = _personas->current().gestures;
  ServoJoint* head = static_cast<ServoJoint*>(_hal->head());
  if (!head) return;

  if (_step == 0) {
    head->moveTo(gestures.lookLeftDeg, gestures.lookSpeedMs, JointEasing::EASE_IN_OUT);
    nextStep();
    return;
  }

  if (_step == 1 && stepElapsed(gestures.lookSpeedMs + 100)) {
    head->moveTo(90, gestures.lookSpeedMs, JointEasing::EASE_IN_OUT);
    nextStep();
    return;
  }

  if (_step == 2 && stepElapsed(gestures.lookSpeedMs + 50)) {
    finishAction(false);
  }
}

void RobotActions::updateLookRight() {
  if (!_hal->head() || !_personas) {
    finishAction(false);
    return;
  }

  const PersonaGestureProfile& gestures = _personas->current().gestures;
  ServoJoint* head = static_cast<ServoJoint*>(_hal->head());
  if (!head) return;

  if (_step == 0) {
    head->moveTo(gestures.lookRightDeg, gestures.lookSpeedMs, JointEasing::EASE_IN_OUT);
    nextStep();
    return;
  }

  if (_step == 1 && stepElapsed(gestures.lookSpeedMs + 100)) {
    head->moveTo(90, gestures.lookSpeedMs, JointEasing::EASE_IN_OUT);
    nextStep();
    return;
  }

  if (_step == 2 && stepElapsed(gestures.lookSpeedMs + 50)) {
    finishAction(false);
  }
}

void RobotActions::updateCelebrate() {
  if (!_personas) return;
  const PersonaGestureProfile& gestures = _personas->current().gestures;
  ServoJoint* leftArm = static_cast<ServoJoint*>(_hal->leftArm());
  ServoJoint* rightArm = static_cast<ServoJoint*>(_hal->rightArm());
  
  if (_step == 0) {
    if (leftArm) leftArm->moveTo(150, 400, JointEasing::EASE_OUT);
    if (rightArm) rightArm->moveTo(150, 400, JointEasing::EASE_OUT);
    nextStep();
    return;
  }

  if (_step == 1) {
    if (!_hal->drive()) return;
    if (stepElapsed(400)) {
      if (_robot && ALLOW_ACTION_DRIVE_MOVEMENT) {
        _robot->move(DriveMode::TURN_LEFT, 220);
      }
      nextStep();
    }
    return;
  }

  if (_step == 2) {
    if (!_hal->drive()) return;
    if (stepElapsed(280)) {
      if (_robot && ALLOW_ACTION_DRIVE_MOVEMENT) {
        _robot->move(DriveMode::TURN_RIGHT, 440);
      }
      nextStep();
    }
    return;
  }

  if (_step == 3) {
    if (stepElapsed(500)) {
      finishAction(true);
    }
  }
}

void RobotActions::updateDance() {
  if (!_personas) return;
  const PersonaGestureProfile& gestures = _personas->current().gestures;
  ServoJoint* leftArm = static_cast<ServoJoint*>(_hal->leftArm());
  ServoJoint* rightArm = static_cast<ServoJoint*>(_hal->rightArm());

  if (_step == 0) {
    if (leftArm) leftArm->moveTo(150, gestures.danceStepMs, JointEasing::EASE_IN_OUT);
    if (rightArm) rightArm->moveTo(150, gestures.danceStepMs, JointEasing::EASE_IN_OUT);
    if (_robot && ALLOW_ACTION_DRIVE_MOVEMENT) {
      _robot->move(DriveMode::TURN_LEFT, 220);
    }
    nextStep();
    return;
  }

  if (_step == 1) {
    if (stepElapsed(gestures.danceStepMs)) {
      if (leftArm) leftArm->moveTo(90, gestures.danceStepMs, JointEasing::EASE_IN_OUT);
      if (rightArm) rightArm->moveTo(90, gestures.danceStepMs, JointEasing::EASE_IN_OUT);
      if (_robot && ALLOW_ACTION_DRIVE_MOVEMENT) {
        _robot->move(DriveMode::TURN_RIGHT, 440);
      }
      nextStep();
    }
    return;
  }

  if (_step == 2) {
    if (stepElapsed(gestures.danceStepMs)) {
      if (leftArm) leftArm->moveTo(150, gestures.danceStepMs, JointEasing::EASE_IN_OUT);
      if (rightArm) rightArm->moveTo(150, gestures.danceStepMs, JointEasing::EASE_IN_OUT);
      if (_robot && ALLOW_ACTION_DRIVE_MOVEMENT) {
        _robot->move(DriveMode::FORWARD, 250);
      }
      nextStep();
    }
    return;
  }

  if (_step == 3 && stepElapsed(gestures.danceStepMs)) {
    finishAction(true);
  }
}

void RobotActions::updateGreet() {
  if (!_hal->head()) {
    finishAction(false);
    return;
  }

  if (_step == 0) {
    ServoJoint* head = static_cast<ServoJoint*>(_hal->head());
    if (head && _personas) {
      head->moveTo(_personas->current().gestures.lookLeftDeg, 400, JointEasing::EASE_IN_OUT);
    } else {
      _hal->head()->moveTo(60);
    }
    nextStep();
    return;
  }

  if (_step == 1 && stepElapsed(300)) {
    // Manually transition to WAVE
    ActionDrivePolicy prevPolicy = drivePolicy(_action);
    _action = ActionId::WAVE;
    _step = 0;
    _stepStartedMs = millis();
    // No need to call stopDrive since GREET to WAVE is PRESERVE_DRIVE to PRESERVE_DRIVE
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
    finishAction(true);
  }
}
