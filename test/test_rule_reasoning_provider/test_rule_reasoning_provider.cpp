#include <unity.h>
#include "RuleReasoningProvider.h"

void setUp(void) {}
void tearDown(void) {}

void test_estop(void) {
    CognitiveContext ctx;
    ctx.correlationId = 101;
    ctx.snapshot.safety.estopped = true;
    
    RuleReasoningProvider provider;
    CognitiveDecision decision = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::SAFETY_BLOCKED), static_cast<uint8_t>(decision.reason));
    TEST_ASSERT_EQUAL_UINT32(101, decision.correlationId);
}

void test_fault(void) {
    CognitiveContext ctx;
    ctx.correlationId = 102;
    ctx.snapshot.safety.faulted = true;
    
    RuleReasoningProvider provider;
    CognitiveDecision decision = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::SAFETY_BLOCKED), static_cast<uint8_t>(decision.reason));
    TEST_ASSERT_EQUAL_UINT32(102, decision.correlationId);
}

void test_boot(void) {
    CognitiveContext ctx;
    ctx.correlationId = 103;
    ctx.snapshot.safety.bootComplete = false;
    
    RuleReasoningProvider provider;
    CognitiveDecision decision = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::OBSERVATION_ONLY), static_cast<uint8_t>(decision.reason));
    TEST_ASSERT_EQUAL_UINT32(103, decision.correlationId);
}

void test_drive_unavailable(void) {
    CognitiveContext ctx;
    ctx.correlationId = 104;
    ctx.snapshot.safety.bootComplete = true;
    ctx.capabilities.drive.capable = true;
    ctx.capabilities.drive.available = false;
    
    RuleReasoningProvider provider;
    CognitiveDecision decision = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::CAPABILITY_UNAVAILABLE), static_cast<uint8_t>(decision.reason));
    TEST_ASSERT_EQUAL_UINT32(104, decision.correlationId);
}

void test_autonomy_disabled(void) {
    CognitiveContext ctx;
    ctx.correlationId = 105;
    ctx.snapshot.safety.bootComplete = true;
    ctx.capabilities.drive.capable = true;
    ctx.capabilities.drive.available = true;
    ctx.snapshot.behavior.autonomyEnabled = false;
    
    RuleReasoningProvider provider;
    CognitiveDecision decision = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::OBSERVATION_ONLY), static_cast<uint8_t>(decision.reason));
    TEST_ASSERT_EQUAL_UINT32(105, decision.correlationId);
}

void test_autonomy_not_permitted(void) {
    CognitiveContext ctx;
    ctx.correlationId = 106;
    ctx.snapshot.safety.bootComplete = true;
    ctx.capabilities.drive.capable = true;
    ctx.capabilities.drive.available = true;
    ctx.snapshot.behavior.autonomyEnabled = true;
    ctx.snapshot.safety.autonomyMotionAllowed = false;
    
    RuleReasoningProvider provider;
    CognitiveDecision decision = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::SAFETY_BLOCKED), static_cast<uint8_t>(decision.reason));
    TEST_ASSERT_EQUAL_UINT32(106, decision.correlationId);
}

void test_safe_intent(void) {
    CognitiveContext ctx;
    ctx.correlationId = 107;
    ctx.snapshot.safety.bootComplete = true;
    ctx.snapshot.safety.estopped = false;
    ctx.snapshot.safety.faulted = false;
    ctx.capabilities.drive.capable = true;
    ctx.capabilities.drive.available = true;
    ctx.capabilities.drive.permitted = true;
    ctx.snapshot.behavior.autonomyEnabled = true;
    ctx.snapshot.safety.autonomyMotionAllowed = true;
    
    RuleReasoningProvider provider;
    CognitiveDecision decision = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::INTENT), static_cast<uint8_t>(decision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::NONE), static_cast<uint8_t>(decision.reason));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentKind::MOVE), static_cast<uint8_t>(decision.intent.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::FORWARD), static_cast<uint8_t>(decision.intent.driveMode));
    TEST_ASSERT_EQUAL_UINT32(250, decision.intent.durationMs);
    TEST_ASSERT_EQUAL_UINT32(107, decision.correlationId);
    TEST_ASSERT_EQUAL_UINT32(107, decision.intent.correlationId);
}

void test_correlation_id(void) {
    CognitiveContext ctx;
    ctx.correlationId = 12345;
    
    // Trigger any rule, e.g. ESTOP to see correlation ID
    ctx.snapshot.safety.estopped = true;

    RuleReasoningProvider provider;
    CognitiveDecision decision = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT32(12345, decision.correlationId);
    
    // For INTENT check inner id too:
    ctx.snapshot.safety.estopped = false;
    ctx.snapshot.safety.faulted = false;
    ctx.snapshot.safety.bootComplete = true;
    ctx.capabilities.drive.capable = true;
    ctx.capabilities.drive.available = true;
    ctx.capabilities.drive.permitted = true;
    ctx.snapshot.behavior.autonomyEnabled = true;
    ctx.snapshot.safety.autonomyMotionAllowed = true;
    
    CognitiveDecision decision2 = provider.reason(ctx);
    TEST_ASSERT_EQUAL_UINT32(12345, decision2.correlationId);
    TEST_ASSERT_EQUAL_UINT32(12345, decision2.intent.correlationId);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_estop);
    RUN_TEST(test_fault);
    RUN_TEST(test_boot);
    RUN_TEST(test_drive_unavailable);
    RUN_TEST(test_autonomy_disabled);
    RUN_TEST(test_autonomy_not_permitted);
    RUN_TEST(test_safe_intent);
    RUN_TEST(test_correlation_id);
    return UNITY_END();
}

void setup() {
    main(0, NULL);
}

void loop() {}
