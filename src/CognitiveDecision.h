#pragma once
#include "CognitiveTypes.h"
#include "IntentTypes.h"

struct CognitiveDecision {
    CognitiveDecisionKind kind = CognitiveDecisionKind::NONE;
    RobotIntent intent;
    CognitiveDecisionReason reason = CognitiveDecisionReason::NONE;
    char intentId[37] = {0};
};

CognitiveDecision makeNoAction(
    CognitiveDecisionReason reason,
    const char* intentId
);

CognitiveDecision makeIntent(
    const RobotIntent& intent,
    const char* intentId
);
