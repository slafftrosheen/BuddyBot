#include "CommandExecutor.h"
#include "ControlRouter.h"

CommandExecutor::CommandExecutor(ControlRouter* router)
    : _router(router) {
}

bool CommandExecutor::execute(const RobotCommand& command) {
    if (!_router) {
        return false;
    }
    if (command.kind == CommandKind::NONE) {
        return false;
    }
    if (command.source != ControlSource::HALO) {
        return false;
    }
    return _router->execute(command);
}
