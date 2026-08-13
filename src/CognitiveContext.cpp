#include "CognitiveContext.h"

CognitiveContext buildCognitiveContext(
    const RuntimeSnapshot& snapshot,
    const RuntimeCapabilities& capabilities,
    uint32_t correlationId
) {
    CognitiveContext ctx;
    ctx.snapshot = snapshot;
    ctx.capabilities = capabilities;
    ctx.correlationId = correlationId;
    return ctx;
}
