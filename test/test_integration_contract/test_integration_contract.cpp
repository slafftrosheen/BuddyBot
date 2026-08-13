#include <unity.h>
#include "RuntimeCapabilities.h"
#include "IntentResolver.h"
#include "CommandExecutor.h"
#include "ControlRouter.h"
#include "RobotAPI.h"
#include "SafetySupervisor.h"
#include "Config.h"

// --- Stubs for ControlRouter dependencies ---
#include "SystemStatus.h"
#include "WifiControl.h"
#include "AutonomyManager.h"

const char* WifiControl::apSsid() const { return ""; }
const char* WifiControl::apIp() const { return ""; }
bool WifiControl::running() const { return false; }
uint8_t WifiControl::clientCount() const { return 0; }
bool WifiControl::controllerPresent() const { return false; }
bool WifiControl::pairingAvailable() const { return false; }
bool WifiControl::start() { return true; }
void WifiControl::stop() {}
bool WifiControl::requestNewPairingCode() { return true; }

void AutonomyManager::setEnabled(bool) {}

void SystemStatus::printStatus() const {}
void SystemStatus::printEvents() const {}

const char* getActiveBuildName() { return "test"; }
void printAllBuildProfiles() {}
void printBuildProfile(const RobotBuildConfig&) {}
FirmwareIdentity getFirmwareIdentity() { return {"","","","","",""}; }

#include "ServoDiagnostics.h"
bool ServoDiagnostics::unlock() { return false; }
const char* ServoDiagnostics::lastResultName() const { return ""; }
void ServoDiagnostics::lock() {}
bool ServoDiagnostics::testWheel(ServoRole, short) { return false; }

// --- RobotAPI Stub ---
static SafetySupervisor* s_robotSafety = nullptr;
static bool s_mockArmResult = true;

bool RobotAPI::driveAvailable() const { return true; }
const ObstacleSafetyStatus& RobotAPI::obstacleSafetyStatus() const { static ObstacleSafetyStatus s; return s; }
const ImuReading& RobotAPI::imuReading() const { static ImuReading r; r.available = true; r.valid = true; return r; }
void RobotAPI::setAutonomyEnabled(bool) {}
void RobotAPI::disarmMotors() {}
void RobotAPI::recordSafetyStop(SafetyStopReason) {}
void RobotAPI::stopAll() {}
bool RobotAPI::armMotors() { return s_mockArmResult; }
void RobotAPI::setMood(Mood, bool) {}
void RobotAPI::nextPersona() {}
void RobotAPI::action(ActionId) {}
void RobotAPI::playExpression(ExpressionId, uint16_t) {}
void RobotAPI::setAttention(AttentionTarget) {}
void RobotAPI::setAccessoryPosition(uint8_t, bool) {}
RangeReading RobotAPI::rangeReading() const { RangeReading r; r.distanceMm=0; r.valid=false; return r; }
const RobotBuildConfig& RobotAPI::buildConfig() const { static RobotBuildConfig c; return c; }
RangeSensorHealth RobotAPI::rangeSensorHealth() const { return RangeSensorHealth::READY; }
uint16_t RobotAPI::rangeConsecutiveInvalid() const { return 0; }
bool RobotAPI::autonomyEnabled() const { return false; }
bool RobotAPI::autonomyMotionAllowed() const { return false; }
ServoDiagnostics* RobotAPI::diagnostics() { static ServoDiagnostics d; return &d; }
void RobotAPI::moveJointTo(ServoRole, int16_t, uint16_t) {}
void RobotAPI::restJoint(ServoRole) {}

bool RobotAPI::move(DriveMode mode, uint16_t durationMs, bool cancelAction, ControlSource source) {
    if (s_robotSafety) {
        if (!s_robotSafety->requestDrive(mode == DriveMode::FORWARD, source == ControlSource::AUTONOMY, 100)) {
            return false;
        }
    }
    return true;
}

// Needed for RuntimeCapabilities
SafetyState RobotAPI::safetyState() const { return s_robotSafety ? s_robotSafety->state() : SafetyState::BOOT; }
SafetyFault RobotAPI::safetyFault() const { return s_robotSafety ? s_robotSafety->fault() : SafetyFault::NONE; }
bool RobotAPI::mayEnableDrive() const { return s_robotSafety ? s_robotSafety->mayEnableDrive() : false; }
bool RobotAPI::mayMoveManipulators() const { return s_robotSafety ? s_robotSafety->mayMoveManipulators() : false; }
ActuatorCapabilities RobotAPI::actuatorCapabilities() const { return ActuatorCapabilities{}; }
void RobotAPI::setSafetySupervisor(SafetySupervisor* s) { s_robotSafety = s; }

// Finally include ControlRouter cpp
#include "../../src/ControlRouter.cpp"


// --- Tests ---
static RobotAPI robot;
static SafetySupervisor safety;
static ControlRouter router;
static CommandExecutor executor(&router);
static IntentResolver resolver;

void setUp(void) {
    s_robotSafety = &safety;
    s_mockArmResult = true;

    SafetyPolicyConfig config;
    safety.configure(config);
    safety.begin();
    
    robot.setSafetySupervisor(&safety);
    router.begin(&robot, nullptr, &safety);

    SafetyInputs inputs;
    inputs.bootComplete = true;
    inputs.hardwareReady = true;
    inputs.driveAvailable = true;
    inputs.imuAvailable = true;
    inputs.imuValid = true;
    safety.completeBoot(inputs, 10);
    
    // Arm the motors by default so MOVE is permitted
    safety.requestArm(inputs, 15);
}

void tearDown(void) {}

void test_integration_happy_path(void) {
    // 1. T0: Capabilities say MOVE is permitted
    RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
    TEST_ASSERT_TRUE(caps.drive.capable);
    if (!ALLOW_MOTOR_ARMING) {
        // If arming is disabled globally, we skip this test or just assert it's false
        TEST_ASSERT_FALSE(caps.drive.permitted);
        return;
    }
    TEST_ASSERT_TRUE(caps.drive.permitted);

    // 2. IntentResolver accepts it
    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    intent.durationMs = 500;
    intent.correlationId = 42;

    IntentResult res = resolver.resolve(intent, caps);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::ACCEPTED), static_cast<uint8_t>(res.result));
    
    // 3. Executor forwards it
    bool executed = executor.execute(res.command);
    TEST_ASSERT_TRUE(executed);
}

void test_integration_arming_disabled_rejects(void) {
    // Mock the state so arming is disabled (e.g. by setting a fault)
    safety.emergencyStop(SafetyFault::CONTROLLER_DISCONNECTED, 20);

    RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
    TEST_ASSERT_FALSE(caps.drive.permitted);

    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    intent.durationMs = 500;

    IntentResult res = resolver.resolve(intent, caps);
    
    // Resolver MUST reject it
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::NOT_PERMITTED), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::NONE), static_cast<uint8_t>(res.command.kind));

    // Executor MUST reject the NONE command
    bool executed = executor.execute(res.command);
    TEST_ASSERT_FALSE(executed);
}

void test_integration_stale_capability_cannot_bypass_safety(void) {
    if (!ALLOW_MOTOR_ARMING) return;

    // T0: Capabilities say MOVE is permitted
    RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
    TEST_ASSERT_TRUE(caps.drive.permitted);

    // IntentResolver creates ACCEPTED command based on T0 state
    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    intent.durationMs = 500;
    IntentResult res = resolver.resolve(intent, caps);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::ACCEPTED), static_cast<uint8_t>(res.result));

    // T1: Safety state changes to ESTOP unexpectedly
    safety.physicalEstop(30);

    // CommandExecutor submits the previously accepted command
    bool executed = executor.execute(res.command);

    // SafetySupervisor MUST reject it despite the command being valid
    TEST_ASSERT_FALSE(executed);
}

void test_integration_safety_exceptions_preserved(void) {
    // Even if drive is not permitted...
    safety.emergencyStop(SafetyFault::IMU_UNAVAILABLE, 20);
    RuntimeCapabilities caps = buildRuntimeCapabilities(robot);
    TEST_ASSERT_FALSE(caps.drive.permitted);

    // STOP intent must still be accepted
    RobotIntent stopIntent;
    stopIntent.kind = IntentKind::STOP;
    IntentResult stopRes = resolver.resolve(stopIntent, caps);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::ACCEPTED), static_cast<uint8_t>(stopRes.result));

    // And it must be executable
    bool stopExecuted = executor.execute(stopRes.command);
    TEST_ASSERT_TRUE(stopExecuted);

    // DISARM intent must still be accepted
    RobotIntent disarmIntent;
    disarmIntent.kind = IntentKind::DISARM;
    IntentResult disarmRes = resolver.resolve(disarmIntent, caps);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::ACCEPTED), static_cast<uint8_t>(disarmRes.result));

    // And it must be executable
    bool disarmExecuted = executor.execute(disarmRes.command);
    TEST_ASSERT_TRUE(disarmExecuted);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_integration_happy_path);
    RUN_TEST(test_integration_arming_disabled_rejects);
    RUN_TEST(test_integration_stale_capability_cannot_bypass_safety);
    RUN_TEST(test_integration_safety_exceptions_preserved);
    return UNITY_END();
}

void setup() {
    main(0, NULL);
}

void loop() {}
