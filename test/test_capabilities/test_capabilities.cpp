#include <unity.h>
#include "RuntimeCapabilities.h"
#include "RobotAPI.h"
#include "SafetySupervisor.h"
#include "Config.h"

// --- RobotAPI Stubs for isolated testing ---
bool RobotAPI::driveAvailable() const { return false; }
SafetyState RobotAPI::safetyState() const { return _safetySupervisor ? _safetySupervisor->state() : SafetyState::BOOT; }
SafetyFault RobotAPI::safetyFault() const { return _safetySupervisor ? _safetySupervisor->fault() : SafetyFault::NONE; }
bool RobotAPI::mayEnableDrive() const { return _safetySupervisor ? _safetySupervisor->mayEnableDrive() : false; }
RangeSensorHealth RobotAPI::rangeSensorHealth() const { return RangeSensorHealth::UNAVAILABLE; }
const ImuReading& RobotAPI::imuReading() const { static ImuReading r; return r; }
const RobotBuildConfig& RobotAPI::buildConfig() const { static RobotBuildConfig c; return c; }
ActuatorCapabilities RobotAPI::actuatorCapabilities() const { return ActuatorCapabilities{}; }
bool RobotAPI::mayMoveManipulators() const { return false; }
bool RobotAPI::autonomyEnabled() const { return false; }
bool RobotAPI::autonomyMotionAllowed() const { return false; }
void RobotAPI::setSafetySupervisor(SafetySupervisor* supervisor) { _safetySupervisor = supervisor; }

SafetySupervisor safety;
RobotAPI robot;

void setUp(void) {
  SafetyPolicyConfig config;
  safety.configure(config);
  safety.begin();
  robot.setSafetySupervisor(&safety);
}

void tearDown(void) {}

void test_capability_drive_safe_default(void) {
  RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
  TEST_ASSERT_TRUE(caps.drive.capable);
  TEST_ASSERT_FALSE(caps.drive.permitted);
  if (!ALLOW_MOTOR_ARMING) {
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::MOTOR_ARMING_DISABLED), static_cast<uint8_t>(caps.drive.reason));
  } else {
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::SAFETY_LOCKED), static_cast<uint8_t>(caps.drive.reason));
  }
}

void test_capability_estop(void) {
  SafetyInputs inputs;
  inputs.bootComplete = true;
  safety.completeBoot(inputs, 100);
  safety.physicalEstop(105);
  
  RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
  
  // ESTOP should lock actions
  TEST_ASSERT_FALSE(caps.actions.permitted);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::ESTOP_ACTIVE), static_cast<uint8_t>(caps.actions.reason));
}

void test_capability_fault(void) {
  SafetyInputs inputs;
  inputs.bootComplete = true;
  safety.completeBoot(inputs, 100);
  safety.emergencyStop(SafetyFault::CONTROLLER_DISCONNECTED, 105);
  
  RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
  
  // FAULT should lock actions
  TEST_ASSERT_FALSE(caps.actions.permitted);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::SAFETY_FAULT), static_cast<uint8_t>(caps.actions.reason));
}

void test_capability_autonomy_safe_defaults(void) {
  RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
  TEST_ASSERT_TRUE(caps.autonomy.capable);
  TEST_ASSERT_FALSE(caps.autonomy.permitted);
  
  if (!ENABLE_AUTONOMY_AT_BOOT && !ENABLE_CAUTIOUS_ROAM && !robot.autonomyEnabled()) {
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::AUTONOMY_DISABLED), static_cast<uint8_t>(caps.autonomy.reason));
  } else {
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::AUTONOMY_NOT_PERMITTED), static_cast<uint8_t>(caps.autonomy.reason));
  }
}

void test_capability_range_sensor_missing(void) {
  // Without initializing RobotHal, the RobotAPI will report missing hardware
  RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
  TEST_ASSERT_TRUE(caps.rangeSensor.capable);
  TEST_ASSERT_FALSE(caps.rangeSensor.available);
  TEST_ASSERT_FALSE(caps.rangeSensor.permitted);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::HARDWARE_MISSING), static_cast<uint8_t>(caps.rangeSensor.reason));
}

int main(int argc, char **argv) {
  UNITY_BEGIN();
  RUN_TEST(test_capability_drive_safe_default);
  RUN_TEST(test_capability_estop);
  RUN_TEST(test_capability_fault);
  RUN_TEST(test_capability_autonomy_safe_defaults);
  RUN_TEST(test_capability_range_sensor_missing);
  return UNITY_END();
}

void setup() {
  main(0, NULL);
}

void loop() {}
