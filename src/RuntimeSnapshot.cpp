#include "RuntimeSnapshot.h"
#include "SafetySupervisor.h"

RuntimeSafetySnapshot buildSafetySnapshot(const SafetySupervisor* supervisor, uint32_t nowMs) {
  RuntimeSafetySnapshot snap;
  if (!supervisor) {
    snap.state = static_cast<uint8_t>(SafetyState::BOOT);
    snap.fault = static_cast<uint8_t>(SafetyFault::BOOT_INCOMPLETE);
    snap.bootComplete = false;
    snap.armed = false;
    snap.estopped = false;
    snap.faulted = false;
    snap.autonomyEnabled = false;
    snap.autonomyMotionAllowed = false;
    snap.stateChangedAtMs = 0;
    snap.manualOverrideUntilMs = 0;
    return snap;
  }

  snap.state = static_cast<uint8_t>(supervisor->state());
  snap.fault = static_cast<uint8_t>(supervisor->fault());
  snap.bootComplete = supervisor->inputs().bootComplete;
  snap.armed = supervisor->state() == SafetyState::ARMED;
  snap.estopped = supervisor->state() == SafetyState::ESTOP;
  snap.faulted = supervisor->state() == SafetyState::FAULT;
  snap.autonomyEnabled = supervisor->autonomyEnabled();
  snap.autonomyMotionAllowed = supervisor->autonomyMotionAllowed(nowMs);
  snap.stateChangedAtMs = supervisor->stateChangedAtMs();
  snap.manualOverrideUntilMs = supervisor->manualOverrideUntilMs();
  return snap;
}
