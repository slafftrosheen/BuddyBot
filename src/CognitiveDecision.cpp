#include "CognitiveDecision.h"
#include <string.h>

CognitiveDecision makeNoAction(CognitiveDecisionReason reason, const char* intentId) {
    CognitiveDecision decision;
    decision.kind = CognitiveDecisionKind::NO_ACTION;
    decision.reason = reason;
    if (intentId) {
        strncpy(decision.intentId, intentId, sizeof(decision.intentId) - 1);
        decision.intentId[sizeof(decision.intentId) - 1] = '\0';
    }
    return decision;
}

CognitiveDecision makeIntent(const RobotIntent& intent, const char* intentId) {
    CognitiveDecision decision;
    decision.kind = CognitiveDecisionKind::INTENT;
    decision.intent = intent;
    if (intentId) {
        strncpy(decision.intent.intentId, intentId, sizeof(decision.intent.intentId) - 1);
        decision.intent.intentId[sizeof(decision.intent.intentId) - 1] = '\0';
        strncpy(decision.intentId, intentId, sizeof(decision.intentId) - 1);
        decision.intentId[sizeof(decision.intentId) - 1] = '\0';
    }
    return decision;
}
