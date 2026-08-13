#include <unity.h>
#include "ReasoningEngine.h"

void setUp(void) {}
void tearDown(void) {}

class FixedDecisionProvider : public ReasoningProvider {
public:
    CognitiveDecision nextDecision;
    CognitiveContext lastContext;

    CognitiveDecision reason(const CognitiveContext& context) override {
        lastContext = context;
        return nextDecision;
    }
};

void test_null_provider(void) {
    ReasoningEngine engine(nullptr);
    CognitiveContext ctx;
    CognitiveDecision outDecision;
    // Set outDecision to something else to verify it gets cleared
    outDecision.kind = CognitiveDecisionKind::INTENT;

    ReasoningResult res = engine.evaluate(ctx, outDecision);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ReasoningResult::PROVIDER_UNAVAILABLE), static_cast<uint8_t>(res));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NONE), static_cast<uint8_t>(outDecision.kind));
}

void test_no_action(void) {
    FixedDecisionProvider provider;
    provider.nextDecision = makeNoAction(CognitiveDecisionReason::NO_ACTION_REQUIRED, "123");
    
    ReasoningEngine engine(&provider);
    CognitiveContext ctx;
    CognitiveDecision outDecision;

    ReasoningResult res = engine.evaluate(ctx, outDecision);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ReasoningResult::SUCCESS), static_cast<uint8_t>(res));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::NO_ACTION), static_cast<uint8_t>(outDecision.kind));
    TEST_ASSERT_EQUAL_STRING("123", outDecision.intentId);
}

void test_valid_intent(void) {
    FixedDecisionProvider provider;
    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    provider.nextDecision = makeIntent(intent, "1234");

    ReasoningEngine engine(&provider);
    CognitiveContext ctx;
    CognitiveDecision outDecision;

    ReasoningResult res = engine.evaluate(ctx, outDecision);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ReasoningResult::SUCCESS), static_cast<uint8_t>(res));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CognitiveDecisionKind::INTENT), static_cast<uint8_t>(outDecision.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentKind::MOVE), static_cast<uint8_t>(outDecision.intent.kind));
    TEST_ASSERT_EQUAL_STRING("1234", outDecision.intentId);
    TEST_ASSERT_EQUAL_STRING("1234", outDecision.intent.intentId);
}

void test_invalid_none_intent(void) {
    FixedDecisionProvider provider;
    RobotIntent intent;
    intent.kind = IntentKind::NONE;
    provider.nextDecision = makeIntent(intent, "555"); // intent IDs match

    ReasoningEngine engine(&provider);
    CognitiveContext ctx;
    CognitiveDecision outDecision;

    ReasoningResult res = engine.evaluate(ctx, outDecision);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ReasoningResult::INVALID_DECISION), static_cast<uint8_t>(res));
}

void test_intent_id_mismatch(void) {
    FixedDecisionProvider provider;
    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    provider.nextDecision = makeIntent(intent, "100");
    // Break the intent ID
    strcpy(provider.nextDecision.intent.intentId, "200");

    ReasoningEngine engine(&provider);
    CognitiveContext ctx;
    CognitiveDecision outDecision;

    ReasoningResult res = engine.evaluate(ctx, outDecision);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ReasoningResult::INVALID_DECISION), static_cast<uint8_t>(res));
}

void test_provider_preserves_context(void) {
    FixedDecisionProvider provider;
    provider.nextDecision = makeNoAction(CognitiveDecisionReason::NO_ACTION_REQUIRED, "1");

    ReasoningEngine engine(&provider);
    CognitiveContext ctx;
    ctx.snapshot.capturedAtMs = 9999;
    ctx.capabilities.drive.capable = true;
    CognitiveDecision outDecision;

    engine.evaluate(ctx, outDecision);

    TEST_ASSERT_EQUAL_UINT32(9999, provider.lastContext.snapshot.capturedAtMs);
    TEST_ASSERT_TRUE(provider.lastContext.capabilities.drive.capable);
}

void test_provider_error(void) {
    FixedDecisionProvider provider;
    // Returns default NONE
    provider.nextDecision.kind = CognitiveDecisionKind::NONE;

    ReasoningEngine engine(&provider);
    CognitiveContext ctx;
    CognitiveDecision outDecision;

    ReasoningResult res = engine.evaluate(ctx, outDecision);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ReasoningResult::NO_DECISION), static_cast<uint8_t>(res));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_null_provider);
    RUN_TEST(test_no_action);
    RUN_TEST(test_valid_intent);
    RUN_TEST(test_invalid_none_intent);
    RUN_TEST(test_intent_id_mismatch);
    RUN_TEST(test_provider_preserves_context);
    RUN_TEST(test_provider_error);
    return UNITY_END();
}

void setup() {
    main(0, NULL);
}

void loop() {}
