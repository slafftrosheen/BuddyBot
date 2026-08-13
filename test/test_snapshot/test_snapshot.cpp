#include <unity.h>
#include "SafetySupervisor.h"
#include "RuntimeSnapshot.h"

SafetySupervisor safety;

void setUp(void) {
  SafetyPolicyConfig config;
  safety.configure(config);
  safety.begin();
}

void tearDown(void) {
}

void test_snapshot_boot(void) {
  RuntimeSafetySnapshot snap = buildSafetySnapshot(&safety, 100);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(SafetyState::BOOT), snap.state);
  TEST_ASSERT_FALSE(snap.armed);
  TEST_ASSERT_FALSE(snap.estopped);
  TEST_ASSERT_FALSE(snap.faulted);
}

void test_snapshot_disarmed(void) {
  SafetyInputs inputs;
  inputs.bootComplete = true;
  inputs.hardwareReady = true;
  inputs.driveAvailable = true;
  inputs.rangeValid = true;
  inputs.forwardMotionBlocked = false;
  inputs.imuAvailable = true;
  inputs.imuValid = true;
  inputs.accelZG = 1.0f; // upright

  safety.completeBoot(inputs, 100);
  safety.update(inputs, 101);

  RuntimeSafetySnapshot snap = buildSafetySnapshot(&safety, 105);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(SafetyState::DISARMED), snap.state);
  TEST_ASSERT_FALSE(snap.armed);
  TEST_ASSERT_FALSE(snap.estopped);
  TEST_ASSERT_FALSE(snap.faulted);
}

void test_snapshot_armed(void) {
  SafetyInputs inputs;
  inputs.bootComplete = true;
  inputs.hardwareReady = true;
  inputs.driveAvailable = true;
  inputs.rangeValid = true;
  inputs.forwardMotionBlocked = false;
  inputs.imuAvailable = true;
  inputs.imuValid = true;
  inputs.accelZG = 1.0f;

  safety.completeBoot(inputs, 100);
  safety.update(inputs, 101);
  safety.requestArm(inputs, 102);

  RuntimeSafetySnapshot snap = buildSafetySnapshot(&safety, 105);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(SafetyState::ARMED), snap.state);
  TEST_ASSERT_TRUE(snap.armed);
  TEST_ASSERT_FALSE(snap.estopped);
  TEST_ASSERT_FALSE(snap.faulted);
}

void test_snapshot_fault(void) {
  SafetyInputs inputs;
  inputs.bootComplete = true;
  inputs.hardwareReady = true;
  inputs.driveAvailable = true;
  inputs.rangeValid = true;
  inputs.forwardMotionBlocked = false;
  inputs.imuAvailable = true;
  inputs.imuValid = true;
  inputs.accelZG = 1.0f;

  safety.completeBoot(inputs, 100);
  safety.update(inputs, 101);
  safety.emergencyStop(SafetyFault::CONTROLLER_DISCONNECTED, 102);

  RuntimeSafetySnapshot snap = buildSafetySnapshot(&safety, 105);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(SafetyState::FAULT), snap.state);
  TEST_ASSERT_FALSE(snap.armed);
  TEST_ASSERT_FALSE(snap.estopped);
  TEST_ASSERT_TRUE(snap.faulted);
}

void test_snapshot_estop(void) {
  SafetyInputs inputs;
  inputs.bootComplete = true;
  inputs.hardwareReady = true;
  inputs.driveAvailable = true;
  inputs.rangeValid = true;
  inputs.forwardMotionBlocked = false;
  inputs.imuAvailable = true;
  inputs.imuValid = true;
  inputs.accelZG = 1.0f;

  safety.completeBoot(inputs, 100);
  safety.update(inputs, 101);
  safety.physicalEstop(102);

  RuntimeSafetySnapshot snap = buildSafetySnapshot(&safety, 105);
  TEST_ASSERT_EQUAL(static_cast<uint8_t>(SafetyState::ESTOP), snap.state);
  TEST_ASSERT_FALSE(snap.armed);
  TEST_ASSERT_TRUE(snap.estopped);
  TEST_ASSERT_FALSE(snap.faulted);
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_snapshot_boot);
  RUN_TEST(test_snapshot_disarmed);
  RUN_TEST(test_snapshot_armed);
  RUN_TEST(test_snapshot_fault);
  RUN_TEST(test_snapshot_estop);
  return UNITY_END();
}

void setup() {
  main(0, NULL);
}

void loop() {
}
