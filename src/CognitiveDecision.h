#pragma once
#include "CognitiveTypes.h"
#include "IntentTypes.h"

struct CognitiveDecision {
    CognitiveDecisionKind kind = CognitiveDecisionKind::NONE;
    RobotIntent intent;
    CognitiveDecisionReason reason = CognitiveDecisionReason::NONE;
    uint32_t correlationId = 0;
};

CognitiveDecision makeNoAction(
    CognitiveDecisionReason reason,
    uint32_t correlationId
);

CognitiveDecision makeIntent(
    const RobotIntent& intent,
    uint32_t correlationId
);
