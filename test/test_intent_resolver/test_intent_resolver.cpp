#include <unity.h>
#include "IntentResolver.h"

IntentResolver resolver;

void setUp(void) {}

void tearDown(void) {}

void test_intent_resolver_move_accepted(void) {
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = true;
    caps.drive.permitted = true;

    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    intent.durationMs = 500;
    intent.correlationId = 12345;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::ACCEPTED), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::MOVE), static_cast<uint8_t>(res.command.kind));
    TEST_ASSERT_EQUAL_UINT32(12345, res.correlationId);
}

void test_intent_resolver_move_not_capable(void) {
    RuntimeCapabilities caps;
    caps.drive.capable = false;

    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    intent.durationMs = 500;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::CAPABILITY_UNAVAILABLE), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::NOT_IMPLEMENTED), static_cast<uint8_t>(res.reason));
}

void test_intent_resolver_move_not_available(void) {
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = false;
    caps.drive.reason = CapabilityReason::HARDWARE_MISSING;

    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    intent.durationMs = 500;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::CAPABILITY_UNAVAILABLE), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::HARDWARE_MISSING), static_cast<uint8_t>(res.reason));
}

void test_intent_resolver_move_not_permitted(void) {
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = true;
    caps.drive.permitted = false;
    caps.drive.reason = CapabilityReason::MOTOR_ARMING_DISABLED;

    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    intent.durationMs = 500;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::NOT_PERMITTED), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::MOTOR_ARMING_DISABLED), static_cast<uint8_t>(res.reason));
}

void test_intent_resolver_move_invalid_parameter(void) {
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = true;
    caps.drive.permitted = true;

    RobotIntent intent;
    intent.kind = IntentKind::MOVE;
    intent.driveMode = DriveMode::FORWARD;
    intent.durationMs = 0; // invalid

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::INVALID_PARAMETER), static_cast<uint8_t>(res.result));
}

void test_intent_resolver_stop_always_accepted(void) {
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = true;
    caps.drive.permitted = false; // Even if not permitted

    RobotIntent intent;
    intent.kind = IntentKind::STOP;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::ACCEPTED), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::STOP), static_cast<uint8_t>(res.command.kind));
}

void test_intent_resolver_disarm_always_accepted(void) {
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = true;
    caps.drive.permitted = false;

    RobotIntent intent;
    intent.kind = IntentKind::DISARM;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::ACCEPTED), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::DISARM), static_cast<uint8_t>(res.command.kind));
}

void test_intent_resolver_action_not_capable(void) {
    RuntimeCapabilities caps;
    caps.actions.capable = false;

    RobotIntent intent;
    intent.kind = IntentKind::ACTION;
    intent.action = ActionId::WAVE;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::CAPABILITY_UNAVAILABLE), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::NOT_IMPLEMENTED), static_cast<uint8_t>(res.reason));
}

void test_intent_resolver_accessory_not_permitted(void) {
    RuntimeCapabilities caps;
    caps.manipulators.capable = true;
    caps.manipulators.available = true;
    caps.manipulators.permitted = false;
    caps.manipulators.reason = CapabilityReason::SAFETY_LOCKED;

    RobotIntent intent;
    intent.kind = IntentKind::ACCESSORY;
    intent.accessoryIndex = 1;
    intent.accessoryActive = true;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::NOT_PERMITTED), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::SAFETY_LOCKED), static_cast<uint8_t>(res.reason));
}

void test_intent_resolver_accessory_not_capable(void) {
    RuntimeCapabilities caps;
    caps.manipulators.capable = false;

    RobotIntent intent;
    intent.kind = IntentKind::ACCESSORY;
    intent.accessoryIndex = 1;
    intent.accessoryActive = true;

    IntentResult res = resolver.resolve(intent, caps);

    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(IntentResolution::CAPABILITY_UNAVAILABLE), static_cast<uint8_t>(res.result));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CapabilityReason::NOT_IMPLEMENTED), static_cast<uint8_t>(res.reason));
}

void test_intent_resolver_correlation_id(void) {
    RuntimeCapabilities caps;
    caps.drive.capable = true;
    caps.drive.available = true;
    caps.drive.permitted = true;

    RobotIntent intent;
    intent.kind = IntentKind::STOP; // universally accepted
    intent.correlationId = 54321;

    IntentResult res = resolver.resolve(intent, caps);
    TEST_ASSERT_EQUAL_UINT32(54321, res.correlationId);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_intent_resolver_move_accepted);
    RUN_TEST(test_intent_resolver_move_not_capable);
    RUN_TEST(test_intent_resolver_move_not_available);
    RUN_TEST(test_intent_resolver_move_not_permitted);
    RUN_TEST(test_intent_resolver_move_invalid_parameter);
    RUN_TEST(test_intent_resolver_stop_always_accepted);
    RUN_TEST(test_intent_resolver_disarm_always_accepted);
    RUN_TEST(test_intent_resolver_action_not_capable);
    RUN_TEST(test_intent_resolver_accessory_not_permitted);
    RUN_TEST(test_intent_resolver_accessory_not_capable);
    RUN_TEST(test_intent_resolver_correlation_id);
    return UNITY_END();
}

void setup() {
    main(0, NULL);
}

void loop() {}
