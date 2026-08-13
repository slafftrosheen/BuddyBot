#include <unity.h>
#include "CommandExecutor.h"
#include "ControlRouter.h"
#include "RobotAPI.h"
#include "SystemStatus.h"
#include "SafetySupervisor.h"

// --- ControlRouter Stub for isolated testing ---
static RobotCommand s_lastCommand;
static bool s_routerExecuteCalled = false;
static bool s_routerExecuteResult = true;

bool ControlRouter::execute(const RobotCommand& cmd) {
    s_lastCommand = cmd;
    s_routerExecuteCalled = true;
    return s_routerExecuteResult;
}

// ControlRouter has other methods in its header. If they are not called, they don't need definitions here.

void setUp(void) {
    s_routerExecuteCalled = false;
    s_routerExecuteResult = true;
    s_lastCommand = RobotCommand{};
}

void tearDown(void) {}

void test_executor_valid_command_forwarded(void) {
    ControlRouter router;
    CommandExecutor executor(&router);

    RobotCommand cmd;
    cmd.kind = CommandKind::STOP;
    cmd.correlationId = 1234;
    cmd.source = ControlSource::AUTONOMY;

    bool result = executor.execute(cmd);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(s_routerExecuteCalled);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::STOP), static_cast<uint8_t>(s_lastCommand.kind));
    TEST_ASSERT_EQUAL_UINT32(1234, s_lastCommand.correlationId);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ControlSource::AUTONOMY), static_cast<uint8_t>(s_lastCommand.source));
}

void test_executor_rejects_none(void) {
    ControlRouter router;
    CommandExecutor executor(&router);

    RobotCommand cmd;
    cmd.kind = CommandKind::NONE;

    bool result = executor.execute(cmd);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(s_routerExecuteCalled);
}

void test_executor_null_router(void) {
    CommandExecutor executor(nullptr);

    RobotCommand cmd;
    cmd.kind = CommandKind::STOP;

    bool result = executor.execute(cmd);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(s_routerExecuteCalled);
}

void test_executor_field_preservation(void) {
    ControlRouter router;
    CommandExecutor executor(&router);

    RobotCommand cmd;
    cmd.kind = CommandKind::MOVE;
    cmd.driveMode = DriveMode::FORWARD;
    cmd.durationMs = 500;
    cmd.correlationId = 9876;
    cmd.source = ControlSource::AUTONOMY;

    bool result = executor.execute(cmd);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(s_routerExecuteCalled);
    
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::MOVE), static_cast<uint8_t>(s_lastCommand.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::FORWARD), static_cast<uint8_t>(s_lastCommand.driveMode));
    TEST_ASSERT_EQUAL_UINT16(500, s_lastCommand.durationMs);
    TEST_ASSERT_EQUAL_UINT32(9876, s_lastCommand.correlationId);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(ControlSource::AUTONOMY), static_cast<uint8_t>(s_lastCommand.source));
}

void test_executor_command_immutability(void) {
    ControlRouter router;
    CommandExecutor executor(&router);

    RobotCommand original;
    original.kind = CommandKind::MOVE;
    original.driveMode = DriveMode::FORWARD;
    original.durationMs = 500;

    RobotCommand copy = original;

    executor.execute(original);

    // Verify original is untouched
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(copy.kind), static_cast<uint8_t>(original.kind));
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(copy.driveMode), static_cast<uint8_t>(original.driveMode));
    TEST_ASSERT_EQUAL_UINT16(copy.durationMs, original.durationMs);
}

void test_executor_router_rejection(void) {
    ControlRouter router;
    CommandExecutor executor(&router);

    s_routerExecuteResult = false; // Simulate router rejection

    RobotCommand cmd;
    cmd.kind = CommandKind::STOP;

    bool result = executor.execute(cmd);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(s_routerExecuteCalled);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_executor_valid_command_forwarded);
    RUN_TEST(test_executor_rejects_none);
    RUN_TEST(test_executor_null_router);
    RUN_TEST(test_executor_field_preservation);
    RUN_TEST(test_executor_command_immutability);
    RUN_TEST(test_executor_router_rejection);
    return UNITY_END();
}

void setup() {
    main(0, NULL);
}

void loop() {}
