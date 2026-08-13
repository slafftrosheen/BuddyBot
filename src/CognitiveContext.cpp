#include "CognitiveContext.h"

CognitiveContext buildCognitiveContext(
    const RuntimeSnapshot& snapshot,
    const RuntimeCapabilities& capabilities
) {
    CognitiveContext ctx;
    ctx.snapshot = snapshot;
    ctx.capabilities = capabilities;
    return ctx;
}
