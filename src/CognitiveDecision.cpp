#include "CognitiveDecision.h"
#include <string.h>

CognitiveDecision makeNoAction(CognitiveDecisionReason reason, const char* authoritativeIntentId) {
    CognitiveDecision decision;
    decision.kind = CognitiveDecisionKind::NO_ACTION;
    decision.reason = reason;
    decision.intentId[0] = '\0';
    if (authoritativeIntentId) {
        strncpy(decision.intentId, authoritativeIntentId, sizeof(decision.intentId) - 1);
        decision.intentId[sizeof(decision.intentId) - 1] = '\0';
    }
    return decision;
}

CognitiveDecision makeIntent(const RobotIntent& intent, const char* authoritativeIntentId) {
    CognitiveDecision decision;
    decision.kind = CognitiveDecisionKind::INTENT;
    decision.intent = intent;
    decision.intent.intentId[0] = '\0';
    decision.intentId[0] = '\0';
    if (authoritativeIntentId) {
        strncpy(decision.intent.intentId, authoritativeIntentId, sizeof(decision.intent.intentId) - 1);
        decision.intent.intentId[sizeof(decision.intent.intentId) - 1] = '\0';
        
        strncpy(decision.intentId, authoritativeIntentId, sizeof(decision.intentId) - 1);
        decision.intentId[sizeof(decision.intentId) - 1] = '\0';
    }
    return decision;
}
