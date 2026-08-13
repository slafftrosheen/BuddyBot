#pragma once

#include "IntentTypes.h"
#include "RuntimeCapabilities.h"
#include "IntentResolver.h"
#include "CommandExecutor.h"

typedef void (*ExecutionResultCallback)(const ExecutionResult&);

class EmbodimentGateway {
public:
    EmbodimentGateway(IntentResolver* resolver, CommandExecutor* executor);
    
    void setExecutionResultCallback(ExecutionResultCallback callback);

    IntentResult submit(const RobotIntent& intent, const RuntimeCapabilities& capabilities);

private:
    IntentResolver* _resolver;
    CommandExecutor* _executor;
    ExecutionResultCallback _callback = nullptr;
    
    void notify(const char* intentId, ExecutionStatus status, const char* reason = nullptr);
};
