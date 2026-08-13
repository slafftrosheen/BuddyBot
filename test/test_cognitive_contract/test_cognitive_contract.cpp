#include <unity.h>
#include "CognitiveContext.h"
#include "CognitiveDecision.h"

// Note: No hardware dependencies are included!
// RobotAPI.h, RobotHal.h, SafetySupervisor.h, ControlRouter.h, CommandExecutor.h, etc. are NOT included.

void setUp(void) {}
void tearDown(void) {}

void test_context_copies_snapshot(void) {
    RuntimeSnapshot snap;
    snap.capturedAtMs = 12345;
    snap.safety.armed = true;
    
    RuntimeCapabilities caps;
    
    CognitiveContext ctx = buildCognitiveContext(snap, caps);
    
    TEST_ASSERT_EQUAL_UINT32(12345, ctx.snapshot.capturedAtMs);
    TEST_ASSERT_TRUE(ctx.snapshot.safety.armed);
}

void test_context_copies_capabilities(void) {
    RuntimeSnapshot snap;
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = false;
    caps.drive.permitted = true;
    caps.drive.reason = CapabilityReason::SAFETY_FAULT;
    
    CognitiveContext ctx = buildCognitiveContext(snap, caps);
    
    TEST_ASSERT_TRUE(ctx.capabilities.drive.capable);
    TEST_ASSERT_FALSE(ctx.capabilities.drive.available);
    TEST_ASSERT_TRUE(ctx.capabilities.drive.permitted);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::SAFETY_FAULT), static_cast<uint8_t>(ctx.capabilities.drive.reason));
}

void test_make_no_action(void) {
    CognitiveDecision decision = makeNoAction(CognitiveDecisionReason::NO_ACTION_REQUIRED, 555);
    
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::NO_ACTION_REQUIRED), static_cast<uint8_t>(decision.reason));
    TEST_ASSERT_EQUAL_UINT32(555, decision.correlationId);
}

void test_make_intent(void) {
    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    
    CognitiveDecision decision = makeIntent(intent, 777);
    
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::INTENT), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentKind::MOVE), static_cast<uint8_t>(decision.intent.kind));
    TEST_ASSERT_EQUAL_UINT32(777, decision.correlationId);
    TEST_ASSERT_EQUAL_UINT32(777, decision.intent.correlationId);
}

void test_correlation_propagation(void) {
    RobotIntent intent;
    intent.kind = IntentKind::ACTION;
    intent.correlationId = 1234; // input intent has some ID
    
    CognitiveDecision decision = makeIntent(intent, 9999);
    
    // Expected: both decision and intent are updated to the final assigned ID
    TEST_ASSERT_EQUAL_UINT32(9999, decision.correlationId);
    TEST_ASSERT_EQUAL_UINT32(9999, decision.intent.correlationId);
}

void test_safety_state_remains_data_only(void) {
    RuntimeSnapshot snap;
    snap.safety.state = 2; // Arbitrary state constant for ESTOP
    RuntimeCapabilities caps;
    
    CognitiveContext ctx = buildCognitiveContext(snap, caps);
    
    TEST_ASSERT_EQUAL_UINT8(2, static_cast<uint8_t>(ctx.snapshot.safety.state));
    // No mutation of SafetySupervisor because it's not even included!
}

void test_capability_state_remains_data_only(void) {
    RuntimeSnapshot snap;
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = true;
    caps.drive.permitted = false;
    caps.drive.reason = CapabilityReason::MOTOR_ARMING_DISABLED;
    
    CognitiveContext ctx = buildCognitiveContext(snap, caps);
    
    TEST_ASSERT_TRUE(ctx.capabilities.drive.capable);
    TEST_ASSERT_TRUE(ctx.capabilities.drive.available);
    TEST_ASSERT_FALSE(ctx.capabilities.drive.permitted);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::MOTOR_ARMING_DISABLED), static_cast<uint8_t>(ctx.capabilities.drive.reason));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_context_copies_snapshot);
    RUN_TEST(test_context_copies_capabilities);
    RUN_TEST(test_make_no_action);
    RUN_TEST(test_make_intent);
    RUN_TEST(test_correlation_propagation);
    RUN_TEST(test_safety_state_remains_data_only);
    RUN_TEST(test_capability_state_remains_data_only);
    return UNITY_END();
}

void setup() {
    main(0, NULL);
}

void loop() {}
