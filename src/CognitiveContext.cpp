#include "CognitiveContext.h"
#include <string.h>

CognitiveContext buildCognitiveContext(
    const RuntimeSnapshot& snapshot,
    const RuntimeCapabilities& capabilities,
    const char* intentId
) {
    CognitiveContext ctx;
    ctx.snapshot = snapshot;
    ctx.capabilities = capabilities;
    if (intentId) {
        strncpy(ctx.intentId, intentId, sizeof(ctx.intentId) - 1);
        ctx.intentId[sizeof(ctx.intentId) - 1] = '\0';
    }
    return ctx;
}
