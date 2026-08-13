import os, re

for root, _, files in os.walk('test'):
    for f in files:
        if f.endswith('.cpp'):
            path = os.path.join(root, f)
            with open(path, 'r') as file:
                content = file.read()
            
            # Use normal string concatenation in replacement strings
            content = re.sub(r'buildCognitiveContext\(([^,]+),\s*([^,]+),\s*(\d+)\)', lambda m: f'buildCognitiveContext({m.group(1)}, {m.group(2)}, "{m.group(3)}")', content)
            content = re.sub(r'makeNoAction\(([^,]+),\s*(\d+)\)', lambda m: f'makeNoAction({m.group(1)}, "{m.group(2)}")', content)
            content = re.sub(r'makeIntent\(([^,]+),\s*(\d+)\)', lambda m: f'makeIntent({m.group(1)}, "{m.group(2)}")', content)
            
            content = re.sub(r'(\w+)\.correlationId\s*=\s*(\d+);', lambda m: f'strcpy({m.group(1)}.intentId, "{m.group(2)}");', content)
            
            content = re.sub(r'TEST_ASSERT_EQUAL_UINT32\((\d+),\s*([\w\.]+)\.correlationId\);', lambda m: f'TEST_ASSERT_EQUAL_STRING("{m.group(1)}", {m.group(2)}.intentId);', content)
            
            content = re.sub(r'executor\.execute\(([^,]+)\)', lambda m: f'executor.execute({m.group(1)}, {m.group(1)}.intentId)', content)
            
            # also test_integration_contract has intent.correlationId = 42
            content = re.sub(r'intent\.correlationId\s*=\s*(\d+);', lambda m: f'strcpy(intent.intentId, "{m.group(1)}");', content)
            
            with open(path, 'w') as file:
                file.write(content)
