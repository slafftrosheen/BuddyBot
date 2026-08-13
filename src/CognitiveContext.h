#pragma once
#include "RuntimeSnapshot.h"
#include "RuntimeCapabilities.h"

struct CognitiveContext {
    RuntimeSnapshot snapshot;
    RuntimeCapabilities capabilities;
    char intentId[37] = {0};
};

CognitiveContext buildCognitiveContext(
    const RuntimeSnapshot& snapshot,
    const RuntimeCapabilities& capabilities,
    const char* intentId = nullptr
);
