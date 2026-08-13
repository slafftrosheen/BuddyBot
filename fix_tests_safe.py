with open('test/test_command_executor/test_command_executor.cpp', 'r') as f:
    text = f.read()

text = text.replace('cmd.correlationId = 1234;', 'strcpy(cmd.intentId, "1234");')
text = text.replace('cmd.correlationId = 9876;', 'strcpy(cmd.intentId, "9876");')
text = text.replace('TEST_ASSERT_EQUAL_UINT32(1234, s_lastCommand.correlationId);', 'TEST_ASSERT_EQUAL_STRING("1234", s_lastCommand.intentId);')
text = text.replace('TEST_ASSERT_EQUAL_UINT32(9876, s_lastCommand.correlationId);', 'TEST_ASSERT_EQUAL_STRING("9876", s_lastCommand.intentId);')

# Fix execute calls
text = text.replace('executor.execute(cmd)', 'executor.execute(cmd, cmd.intentId)')
text = text.replace('executor.execute(original)', 'executor.execute(original, original.intentId)')

with open('test/test_command_executor/test_command_executor.cpp', 'w') as f:
    f.write(text)

with open('test/test_integration_contract/test_integration_contract.cpp', 'r') as f:
    text = f.read()
text = text.replace('intent.correlationId = 42;', 'strcpy(intent.intentId, "42");')
with open('test/test_integration_contract/test_integration_contract.cpp', 'w') as f:
    f.write(text)

with open('test/test_reasoning_engine/test_reasoning_engine.cpp', 'r') as f:
    text = f.read()
text = text.replace('provider.nextDecision.correlationId = 123;', 'strcpy(provider.nextDecision.intentId, "123");')
text = text.replace('provider.nextDecision.correlationId = 1234;', 'strcpy(provider.nextDecision.intentId, "1234");')
text = text.replace('provider.nextDecision.intent.correlationId = 1234;', 'strcpy(provider.nextDecision.intent.intentId, "1234");')
text = text.replace('provider.nextDecision.intent.correlationId = 200;', 'strcpy(provider.nextDecision.intent.intentId, "200");')
text = text.replace('TEST_ASSERT_EQUAL_UINT32(123, outDecision.correlationId);', 'TEST_ASSERT_EQUAL_STRING("123", outDecision.intentId);')
text = text.replace('TEST_ASSERT_EQUAL_UINT32(1234, outDecision.correlationId);', 'TEST_ASSERT_EQUAL_STRING("1234", outDecision.intentId);')
text = text.replace('TEST_ASSERT_EQUAL_UINT32(1234, outDecision.intent.correlationId);', 'TEST_ASSERT_EQUAL_STRING("1234", outDecision.intent.intentId);')

with open('test/test_reasoning_engine/test_reasoning_engine.cpp', 'w') as f:
    f.write(text)
