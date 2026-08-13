#pragma once

#include "IntentTypes.h"
#include "RuntimeCapabilities.h"

/*
 * IntentResolver acceptance is only preflight validation.
 * SafetySupervisor remains the authoritative execution gate.
 * Runtime state may change between resolution and execution.
 */
class IntentResolver {
public:
    IntentResult resolve(
        const RobotIntent& intent,
        const RuntimeCapabilities& capabilities
    ) const;
};
