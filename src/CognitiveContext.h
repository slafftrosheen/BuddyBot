#pragma once
#include "RuntimeSnapshot.h"
#include "RuntimeCapabilities.h"

struct CognitiveContext {
    RuntimeSnapshot snapshot;
    RuntimeCapabilities capabilities;
    uint32_t correlationId = 0;
};

CognitiveContext buildCognitiveContext(
    const RuntimeSnapshot& snapshot,
    const RuntimeCapabilities& capabilities,
    uint32_t correlationId
);
