#include <Arduino.h>
#include <unity.h>

#include "ControlProtocol.h"
#include "WebControlProtocol.h"

void test_serial_pipe_move_is_parsed() {
  ControlProtocol protocol;
  RobotCommand command = {};

  TEST_ASSERT_TRUE(protocol.parseLine("CMD|MOVE|FWD|250", command));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::MOVE), static_cast<uint8_t>(command.kind));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(DriveMode::FORWARD), static_cast<uint8_t>(command.driveMode));
  TEST_ASSERT_EQUAL_UINT16(250, command.durationMs);
}

void test_serial_parser_rejects_unknown_command() {
  ControlProtocol protocol;
  RobotCommand command = {};

  TEST_ASSERT_FALSE(protocol.parseLine("CMD|MOVE|UP|250", command));
}

void test_web_move_requires_a_token() {
  WebControlProtocol protocol;
  WebParsedMessage message = {};
  WebProtocolError error = WebProtocolError::NONE;
  const char payload[] = R"({"v":1,"id":1,"type":"move","mode":"forward"})";

  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(payload), strlen(payload), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::INVALID_ARGUMENT), static_cast<uint8_t>(error));
}

void test_web_pairing_accepts_display_spacing() {
  WebControlProtocol protocol;
  WebParsedMessage message = {};
  WebProtocolError error = WebProtocolError::NONE;
  const char payload[] = R"({"v":1,"type":"pair","code":"1234 5678"})";

  TEST_ASSERT_TRUE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(payload), strlen(payload), message, error));
  TEST_ASSERT_EQUAL_STRING("12345678", message.pairingDigits);
}

void test_web_parser_rejects_unsupported_version() {
  WebControlProtocol protocol;
  WebParsedMessage message = {};
  WebProtocolError error = WebProtocolError::NONE;
  const char payload[] = R"({"v":2,"type":"hello"})";

  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(payload), strlen(payload), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::UNSUPPORTED_VERSION), static_cast<uint8_t>(error));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_serial_pipe_move_is_parsed);
  RUN_TEST(test_serial_parser_rejects_unknown_command);
  RUN_TEST(test_web_move_requires_a_token);
  RUN_TEST(test_web_pairing_accepts_display_spacing);
  RUN_TEST(test_web_parser_rejects_unsupported_version);
  UNITY_END();
}

void loop() {}
