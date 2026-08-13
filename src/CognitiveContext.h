#pragma once
#include "RuntimeSnapshot.h"
#include "RuntimeCapabilities.h"

struct CognitiveContext {
    RuntimeSnapshot snapshot;
    RuntimeCapabilities capabilities;
};

CognitiveContext buildCognitiveContext(
    const RuntimeSnapshot& snapshot,
    const RuntimeCapabilities& capabilities
);
