#pragma once
#include "ReasoningProvider.h"

class RuleReasoningProvider : public ReasoningProvider {
public:
    CognitiveDecision reason(const CognitiveContext& context) override;
};
