#include <unity.h>

#include "SafetySupervisor.h"

namespace {
SafetyInputs safeInputs() {
  SafetyInputs inputs;
  inputs.bootComplete = true;
  inputs.hardwareReady = true;
  inputs.driveAvailable = true;
  inputs.rangeValid = true;
  inputs.forwardMotionBlocked = false;
  inputs.imuAvailable = true;
  inputs.imuValid = true;
  inputs.imuSampleTimeMs = 100;
  inputs.accelXG = 0.0f;
  inputs.accelYG = 0.0f;
  inputs.accelZG = 1.0f;
  inputs.gyroXDps = 0.0f;
  inputs.gyroYDps = 0.0f;
  inputs.gyroZDps = 0.0f;
  return inputs;
}
}

void test_supervisor_requires_boot_and_explicit_arm() {
  SafetySupervisor supervisor;
  SafetyInputs inputs = safeInputs();

  TEST_ASSERT_FALSE(supervisor.requestArm(inputs, 100));

  supervisor.completeBoot(inputs, 100);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SafetyState::DISARMED), static_cast<uint8_t>(supervisor.state()));
  TEST_ASSERT_FALSE(supervisor.requestDrive(false, false, 110));
  TEST_ASSERT_TRUE(supervisor.requestArm(inputs, 120));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SafetyState::ARMED), static_cast<uint8_t>(supervisor.state()));
}

void test_forward_sensor_failure_latches_fault_and_disarm_request() {
  SafetySupervisor supervisor;
  SafetyInputs inputs = safeInputs();
  supervisor.completeBoot(inputs, 100);
  TEST_ASSERT_TRUE(supervisor.requestArm(inputs, 110));

  inputs.rangeValid = false;
  supervisor.update(inputs, 120);
  TEST_ASSERT_FALSE(supervisor.requestDrive(true, false, 120));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SafetyState::FAULT), static_cast<uint8_t>(supervisor.state()));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SafetyFault::RANGE_SENSOR_INVALID), static_cast<uint8_t>(supervisor.fault()));
  TEST_ASSERT_TRUE(supervisor.consumeDisarmRequest());
}

void test_manual_control_preempts_autonomy_until_lease_expires() {
  SafetySupervisor supervisor;
  SafetyInputs inputs = safeInputs();
  supervisor.completeBoot(inputs, 100);
  TEST_ASSERT_TRUE(supervisor.requestArm(inputs, 110));
  TEST_ASSERT_TRUE(supervisor.requestAutonomy(true, 120));
  TEST_ASSERT_TRUE(supervisor.requestDrive(false, false, 130));
  TEST_ASSERT_FALSE(supervisor.requestDrive(false, true, 200));
  inputs.imuSampleTimeMs = 800;
  supervisor.update(inputs, 800);
  TEST_ASSERT_TRUE(supervisor.requestDrive(false, true, 800));
}

void test_physical_estop_requires_local_reset_before_rearm() {
  SafetySupervisor supervisor;
  SafetyInputs inputs = safeInputs();
  supervisor.completeBoot(inputs, 100);
  TEST_ASSERT_TRUE(supervisor.requestArm(inputs, 110));

  supervisor.physicalEstop(120);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SafetyState::ESTOP), static_cast<uint8_t>(supervisor.state()));
  TEST_ASSERT_FALSE(supervisor.requestArm(inputs, 130));
  TEST_ASSERT_TRUE(supervisor.clearPhysicalEstop(inputs, 140));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(SafetyState::DISARMED), static_cast<uint8_t>(supervisor.state()));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_supervisor_requires_boot_and_explicit_arm);
  RUN_TEST(test_forward_sensor_failure_latches_fault_and_disarm_request);
  RUN_TEST(test_manual_control_preempts_autonomy_until_lease_expires);
  RUN_TEST(test_physical_estop_requires_local_reset_before_rearm);
  UNITY_END();
}

void loop() {}
