#include "EmbodimentGateway.h"

EmbodimentGateway::EmbodimentGateway(IntentResolver* resolver, CommandExecutor* executor)
    : _resolver(resolver), _executor(executor) {
}

void EmbodimentGateway::setExecutionResultCallback(ExecutionResultCallback callback) {
    _callback = callback;
}

void EmbodimentGateway::notify(const char* intentId, ExecutionStatus status, const char* reason) {
    if (_callback) {
        ExecutionResult result;
        strncpy(result.intentId, intentId, sizeof(result.intentId) - 1);
        result.intentId[sizeof(result.intentId) - 1] = '\0';
        result.status = status;
        if (reason) {
            strncpy(result.reason, reason, sizeof(result.reason) - 1);
            result.reason[sizeof(result.reason) - 1] = '\0';
        } else {
            result.reason[0] = '\0';
        }
        _callback(result);
    }
}

IntentResult EmbodimentGateway::submit(const RobotIntent& intent, const RuntimeCapabilities& capabilities) {
    if (!_resolver || !_executor) {
        IntentResult res;
        res.result = IntentResolution::INVALID_INTENT;
        res.reason = CapabilityReason::NONE;
        res.command.kind = CommandKind::NONE;
        return res;
    }

    notify(intent.intentId, ExecutionStatus::RESOLVING);
    IntentResult res = _resolver->resolve(intent, capabilities);

    if (res.result == IntentResolution::ACCEPTED) {
        notify(intent.intentId, ExecutionStatus::ACCEPTED);
        
        // Command execution determines if it was authorized and succeeded
        // The CommandExecutor will internally consult the SafetySupervisor
        // and notify EXECUTING / SUCCEEDED / DENIED / FAILED if we pass the callback
        // For now, CommandExecutor returns bool. We can adapt this.
        
        // Wait, the executor doesn't know about intentId yet. We need to pass intentId.
        // Let's copy the intentId to the result so it can be used.
        strncpy(res.intentId, intent.intentId, sizeof(res.intentId) - 1);
        res.intentId[sizeof(res.intentId) - 1] = '\0';
        
        ExecutionResult execResult = _executor->execute(res.command, intent.intentId);
        notify(intent.intentId, execResult.status, execResult.reason);
    } else {
        notify(intent.intentId, ExecutionStatus::REJECTED, "IntentResolutionFailed");
        strncpy(res.intentId, intent.intentId, sizeof(res.intentId) - 1);
        res.intentId[sizeof(res.intentId) - 1] = '\0';
    }

    return res;
}
