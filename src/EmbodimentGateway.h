#pragma once

#include "IntentTypes.h"
#include "RuntimeCapabilities.h"
#include "IntentResolver.h"
#include "CommandExecutor.h"

class EmbodimentGateway {
public:
    EmbodimentGateway(IntentResolver* resolver, CommandExecutor* executor);

    IntentResult submit(const RobotIntent& intent, const RuntimeCapabilities& capabilities);

private:
    IntentResolver* _resolver;
    CommandExecutor* _executor;
};
