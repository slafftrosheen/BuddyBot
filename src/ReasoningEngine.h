#pragma once
#include "ReasoningTypes.h"
#include "ReasoningProvider.h"

class ReasoningEngine {
public:
    explicit ReasoningEngine(ReasoningProvider* provider);

    ReasoningResult evaluate(
        const CognitiveContext& context,
        CognitiveDecision& decision
    );

private:
    ReasoningProvider* _provider;
};
