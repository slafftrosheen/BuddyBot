#include "CommandExecutor.h"
#include "ControlRouter.h"
#include <string.h>

CommandExecutor::CommandExecutor(ControlRouter* router)
    : _router(router) {
}

ExecutionResult CommandExecutor::execute(const RobotCommand& command, const char* intentId) {
    ExecutionResult res;
    res.intentId[0] = '\0';
    if (intentId) {
        strncpy(res.intentId, intentId, sizeof(res.intentId) - 1);
        res.intentId[sizeof(res.intentId) - 1] = '\0';
    }

    if (!_router) {
        res.status = ExecutionStatus::FAILED;
        strncpy(res.reason, "NO_ROUTER", sizeof(res.reason) - 1);
        res.reason[sizeof(res.reason) - 1] = '\0';
        return res;
    }
    if (command.kind == CommandKind::NONE) {
        res.status = ExecutionStatus::REJECTED;
        strncpy(res.reason, "INVALID_COMMAND", sizeof(res.reason) - 1);
        res.reason[sizeof(res.reason) - 1] = '\0';
        return res;
    }
    if (command.source != ControlSource::HALO) {
        res.status = ExecutionStatus::REJECTED;
        strncpy(res.reason, "INVALID_SOURCE", sizeof(res.reason) - 1);
        res.reason[sizeof(res.reason) - 1] = '\0';
        return res;
    }
    
    bool success = _router->execute(command);
    if (success) {
        res.status = ExecutionStatus::SUCCEEDED;
        res.reason[0] = '\0';
    } else {
        SafetyFault fault = _router->safetyFault();
        if (fault != SafetyFault::NONE) {
            res.status = ExecutionStatus::DENIED;
            const char* faultStr = SafetySupervisor::faultName(fault);
            strncpy(res.reason, faultStr, sizeof(res.reason) - 1);
            res.reason[sizeof(res.reason) - 1] = '\0';
        } else {
            res.status = ExecutionStatus::FAILED;
            strncpy(res.reason, "EXECUTION_FAILED", sizeof(res.reason) - 1);
            res.reason[sizeof(res.reason) - 1] = '\0';
        }
    }
    return res;
}
