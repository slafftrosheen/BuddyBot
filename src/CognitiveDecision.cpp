#include "CognitiveDecision.h"

CognitiveDecision makeNoAction(CognitiveDecisionReason reason, uint32_t correlationId) {
    CognitiveDecision decision;
    decision.kind = CognitiveDecisionKind::NO_ACTION;
    decision.reason = reason;
    decision.correlationId = correlationId;
    return decision;
}

CognitiveDecision makeIntent(const RobotIntent& intent, uint32_t correlationId) {
    CognitiveDecision decision;
    decision.kind = CognitiveDecisionKind::INTENT;
    decision.intent = intent;
    decision.intent.correlationId = correlationId; // Enforce propagation
    decision.correlationId = correlationId;
    return decision;
}
