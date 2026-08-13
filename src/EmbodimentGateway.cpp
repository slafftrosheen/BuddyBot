#include "EmbodimentGateway.h"

EmbodimentGateway::EmbodimentGateway(IntentResolver* resolver, CommandExecutor* executor)
    : _resolver(resolver), _executor(executor) {
}

IntentResult EmbodimentGateway::submit(const RobotIntent& intent, const RuntimeCapabilities& capabilities) {
    if (!_resolver || !_executor) {
        IntentResult res;
        res.result = IntentResolution::INVALID_INTENT;
        res.reason = CapabilityReason::NONE;
        res.command.kind = CommandKind::NONE;
        return res;
    }

    IntentResult res = _resolver->resolve(intent, capabilities);

    if (res.result == IntentResolution::ACCEPTED) {
        _executor->execute(res.command);
    }

    return res;
}
