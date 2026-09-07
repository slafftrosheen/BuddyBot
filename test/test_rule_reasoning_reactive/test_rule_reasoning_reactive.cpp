#include <unity.h>
#include "RuleReasoningProvider.h"

void setUp(void) {}
void tearDown(void) {}

// Helper: build a context where all safety checks pass and drive is permitted
static CognitiveContext makeReadyContext(const char* id, uint16_t rangeMm, bool rangeValid = true, bool rangeAvailable = true) {
    CognitiveContext ctx;
    strncpy(ctx.intentId, id, sizeof(ctx.intentId) - 1);
    ctx.intentId[sizeof(ctx.intentId) - 1] = '\0';
    ctx.snapshot.safety.bootComplete = true;
    ctx.snapshot.safety.estopped = false;
    ctx.snapshot.safety.faulted = false;
    ctx.snapshot.safety.autonomyMotionAllowed = true;
    ctx.snapshot.behavior.autonomyEnabled = true;
    ctx.capabilities.drive.capable = true;
    ctx.capabilities.drive.available = true;
    ctx.capabilities.drive.permitted = true;
    ctx.snapshot.range.available = rangeAvailable;
    ctx.snapshot.range.valid = rangeValid;
    ctx.snapshot.range.distanceMm = rangeMm;
    return ctx;
}

void test_clear_path_moves_forward(void) {
    CognitiveContext ctx = makeReadyContext("R01", 500);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::INTENT), static_cast<uint8_t>(d.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentKind::MOVE), static_cast<uint8_t>(d.intent.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::FORWARD), static_cast<uint8_t>(d.intent.driveMode));
    TEST_ASSERT_EQUAL_UINT32(250, d.intent.durationMs);
}

void test_caution_zone_turns(void) {
    // Range between OBSTACLE_EMERGENCY_STOP_MM and OBSTACLE_CAUTION_MM -> TURN
    CognitiveContext ctx = makeReadyContext("R02", 180);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::INTENT), static_cast<uint8_t>(d.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentKind::MOVE), static_cast<uint8_t>(d.intent.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::TURN_RIGHT), static_cast<uint8_t>(d.intent.driveMode));
}

void test_too_close_reverses(void) {
    // Range below OBSTACLE_EMERGENCY_STOP_MM -> REVERSE
    CognitiveContext ctx = makeReadyContext("R03", 50);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::INTENT), static_cast<uint8_t>(d.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentKind::MOVE), static_cast<uint8_t>(d.intent.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::REVERSE), static_cast<uint8_t>(d.intent.driveMode));
}

void test_range_invalid_no_action(void) {
    CognitiveContext ctx = makeReadyContext("R04", 0, false, true);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(d.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::OBSERVATION_ONLY), static_cast<uint8_t>(d.reason));
}

void test_range_unavailable_no_action(void) {
    CognitiveContext ctx = makeReadyContext("R05", 0, false, false);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(d.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionReason::OBSERVATION_ONLY), static_cast<uint8_t>(d.reason));
}

void test_boundary_at_emergency_stop(void) {
    // Exactly at boundary — should be REVERSE (< threshold)
    CognitiveContext ctx = makeReadyContext("R06", 119);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::REVERSE), static_cast<uint8_t>(d.intent.driveMode));
}

void test_boundary_at_caution(void) {
    // Exactly at caution boundary — should be TURN (< threshold)
    CognitiveContext ctx = makeReadyContext("R07", 219);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::TURN_RIGHT), static_cast<uint8_t>(d.intent.driveMode));
}

void test_boundary_at_clear(void) {
    // At exact caution threshold -> FORWARD
    CognitiveContext ctx = makeReadyContext("R08", 220);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::FORWARD), static_cast<uint8_t>(d.intent.driveMode));
}

void test_intent_ids_propagated(void) {
    CognitiveContext ctx = makeReadyContext("TRACE-42", 300);
    RuleReasoningProvider provider;
    CognitiveDecision d = provider.reason(ctx);

    TEST_ASSERT_EQUAL_STRING("TRACE-42", d.intentId);
    TEST_ASSERT_EQUAL_STRING("TRACE-42", d.intent.intentId);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_clear_path_moves_forward);
    RUN_TEST(test_caution_zone_turns);
    RUN_TEST(test_too_close_reverses);
    RUN_TEST(test_range_invalid_no_action);
    RUN_TEST(test_range_unavailable_no_action);
    RUN_TEST(test_boundary_at_emergency_stop);
    RUN_TEST(test_boundary_at_caution);
    RUN_TEST(test_boundary_at_clear);
    RUN_TEST(test_intent_ids_propagated);
    return UNITY_END();
}

void setup() {
    main(0, NULL);
}

void loop() {}
