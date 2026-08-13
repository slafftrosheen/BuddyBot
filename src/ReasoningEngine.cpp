#include "ReasoningEngine.h"

ReasoningEngine::ReasoningEngine(ReasoningProvider* provider)
    : _provider(provider) {}

ReasoningResult ReasoningEngine::evaluate(
    const CognitiveContext& context,
    CognitiveDecision& decision
) {
    // 1. Clear output decision to a known NONE state.
    decision = CognitiveDecision();

    // 2. If provider == nullptr: return PROVIDER_UNAVAILABLE
    if (!_provider) {
        return ReasoningResult::PROVIDER_UNAVAILABLE;
    }

    // 3. Call: provider->reason(context)
    CognitiveDecision candidate = _provider->reason(context);

    // 4. Validate returned decision.
    if (candidate.kind == CognitiveDecisionKind::NONE) {
        return ReasoningResult::NO_DECISION;
    }

    if (candidate.kind != CognitiveDecisionKind::NO_ACTION && 
        candidate.kind != CognitiveDecisionKind::INTENT) {
        return ReasoningResult::INVALID_DECISION;
    }

    if (candidate.kind == CognitiveDecisionKind::INTENT) {
        if (candidate.intent.kind == IntentKind::NONE) {
            return ReasoningResult::INVALID_DECISION;
        }
        if (candidate.correlationId != candidate.intent.correlationId) {
            return ReasoningResult::INVALID_DECISION;
        }
    }

    // 5. Copy valid decision to output.
    decision = candidate;

    // 6. Return SUCCESS.
    return ReasoningResult::SUCCESS;
}
