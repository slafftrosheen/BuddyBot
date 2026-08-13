#pragma once
#include "CognitiveContext.h"
#include "CognitiveDecision.h"

class ReasoningProvider {
public:
    virtual ~ReasoningProvider() = default;

    virtual CognitiveDecision reason(
        const CognitiveContext& context
    ) = 0;
};
