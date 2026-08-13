#pragma once
#include "ControlTypes.h"
#include "ControlRouter.h"
#include "IntentTypes.h"

class ControlRouter;

class CommandExecutor {
public:
    explicit CommandExecutor(ControlRouter* router);

    ExecutionResult execute(const RobotCommand& command, const char* intentId);

private:
    ControlRouter* _router;
};
