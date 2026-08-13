#include <Arduino.h>
#include <unity.h>

#include "ControlProtocol.h"
#include "WebControlProtocol.h"
#include "RuntimeSnapshot.h"

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

void test_serial_pipe_parser_rejects_extra_fields_and_invalid_duration() {
  ControlProtocol protocol;
  RobotCommand command = {};

  TEST_ASSERT_FALSE(protocol.parseLine("CMD|MOVE|FWD|250|unexpected", command));
  TEST_ASSERT_FALSE(protocol.parseLine("CMD|MOVE|FWD|invalid", command));
  TEST_ASSERT_FALSE(protocol.parseLine("CMD|MOVE|FWD|0", command));
}

void test_serial_imu_status_is_parsed() {
  ControlProtocol protocol;
  RobotCommand command = {};

  TEST_ASSERT_TRUE(protocol.parseLine("IMU STATUS", command));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::CTRL_IMU_STATUS), static_cast<uint8_t>(command.kind));
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

void test_web_parser_rejects_unknown_fields() {
  WebControlProtocol protocol;
  WebParsedMessage message = {};
  WebProtocolError error = WebProtocolError::NONE;
  const char payload[] = R"({"v":1,"id":1,"type":"hello","unexpected":true})";

  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(payload), strlen(payload), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::INVALID_ARGUMENT), static_cast<uint8_t>(error));
}

void test_web_move_requires_bounded_duration() {
  WebControlProtocol protocol;
  WebParsedMessage message = {};
  WebProtocolError error = WebProtocolError::NONE;
  const char missingDuration[] = R"({"v":1,"id":1,"type":"move","mode":"forward","token":"0123456789abcdef0123456789abcdef"})";
  const char excessiveDuration[] = R"({"v":1,"id":2,"type":"move","mode":"forward","durationMs":251,"token":"0123456789abcdef0123456789abcdef"})";

  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(missingDuration), strlen(missingDuration), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::INVALID_ARGUMENT), static_cast<uint8_t>(error));
  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(excessiveDuration), strlen(excessiveDuration), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::INVALID_ARGUMENT), static_cast<uint8_t>(error));
}

void test_web_parser_rejects_invalid_mood_and_pairing_suffix() {
  WebControlProtocol protocol;
  WebParsedMessage message = {};
  WebProtocolError error = WebProtocolError::NONE;
  const char invalidMood[] = R"({"v":1,"id":1,"type":"mood","mood":"dangerous","token":"0123456789abcdef0123456789abcdef"})";
  const char extraPairingDigit[] = R"({"v":1,"type":"pair","code":"1234 56789"})";

  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(invalidMood), strlen(invalidMood), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::INVALID_ARGUMENT), static_cast<uint8_t>(error));
  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(extraPairingDigit), strlen(extraPairingDigit), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::INVALID_ARGUMENT), static_cast<uint8_t>(error));
}

void test_web_state_parsing() {
  WebControlProtocol protocol;
  WebParsedMessage message = {};
  WebProtocolError error = WebProtocolError::NONE;

  // A. Basic STATE
  const char basicState[] = R"({"v":1,"id":42,"type":"state"})";
  TEST_ASSERT_TRUE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(basicState), strlen(basicState), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebMessageType::STATE), static_cast<uint8_t>(message.type));
  TEST_ASSERT_EQUAL_UINT32(42, message.requestId);
  // F. Verify STATE does NOT become a command
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(CommandKind::NONE), static_cast<uint8_t>(message.command.kind));

  // B. With valid token
  const char tokenState[] = R"({"v":1,"id":42,"type":"state","token":"0123456789abcdef0123456789abcdef"})";
  message = {};
  TEST_ASSERT_TRUE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(tokenState), strlen(tokenState), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebMessageType::STATE), static_cast<uint8_t>(message.type));
  TEST_ASSERT_TRUE(message.hasToken);
  TEST_ASSERT_EQUAL_STRING("0123456789abcdef0123456789abcdef", message.token);

  // C. Unknown field
  const char evilState[] = R"({"v":1,"id":42,"type":"state","evil":true})";
  message = {};
  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(evilState), strlen(evilState), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::INVALID_ARGUMENT), static_cast<uint8_t>(error));

  // D. Wrong version
  const char wrongVersion[] = R"({"v":2,"id":42,"type":"state"})";
  message = {};
  TEST_ASSERT_FALSE(protocol.parseCommand(reinterpret_cast<const uint8_t*>(wrongVersion), strlen(wrongVersion), message, error));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(WebProtocolError::UNSUPPORTED_VERSION), static_cast<uint8_t>(error));
}

void test_web_state_serialization() {
  WebControlProtocol protocol;
  RuntimeSnapshot snapshot{};
  snapshot.capturedAtMs = 1000;
  snapshot.safety.state = 2; // ARMED
  snapshot.safety.fault = 0;
  snapshot.safety.bootComplete = true;
  snapshot.safety.armed = true;
  snapshot.drive.available = true;
  snapshot.drive.active = true;
  snapshot.range.distanceMm = 850;
  snapshot.imu.accelZG = 0.99f;
  snapshot.behavior.action = 0;
  snapshot.hardware.servoBusPresent = true;

  String json = protocol.generateRuntimeSnapshot(123, snapshot);

  // E. Verify JSON contains essential substrings (basic check)
  TEST_ASSERT_TRUE(json.indexOf("\"v\":1") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"id\":123") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"type\":\"state\"") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"ok\":true") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"state\":{") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"capturedAtMs\":1000") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"safety\":{") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"drive\":{") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"range\":{") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"imu\":{") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"behavior\":{") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"hardware\":{") >= 0);
  TEST_ASSERT_TRUE(json.indexOf("\"servoBusPresent\":true") >= 0);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_serial_pipe_move_is_parsed);
  RUN_TEST(test_serial_parser_rejects_unknown_command);
  RUN_TEST(test_serial_pipe_parser_rejects_extra_fields_and_invalid_duration);
  RUN_TEST(test_serial_imu_status_is_parsed);
  RUN_TEST(test_web_move_requires_a_token);
  RUN_TEST(test_web_pairing_accepts_display_spacing);
  RUN_TEST(test_web_parser_rejects_unsupported_version);
  RUN_TEST(test_web_parser_rejects_unknown_fields);
  RUN_TEST(test_web_move_requires_bounded_duration);
  RUN_TEST(test_web_parser_rejects_invalid_mood_and_pairing_suffix);
  RUN_TEST(test_web_state_parsing);
  RUN_TEST(test_web_state_serialization);
  UNITY_END();
}

void loop() {}
