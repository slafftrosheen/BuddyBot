#pragma once
#include "ControlTypes.h"

class ControlRouter;

class CommandExecutor {
public:
    explicit CommandExecutor(ControlRouter* router);

    bool execute(const RobotCommand& command);

private:
    ControlRouter* _router;
};
