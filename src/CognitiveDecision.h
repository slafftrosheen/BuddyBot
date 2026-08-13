#pragma once
#include "CognitiveTypes.h"
#include "IntentTypes.h"

struct CognitiveDecision {
    CognitiveDecisionKind kind = CognitiveDecisionKind::NONE;
    RobotIntent intent;
    CognitiveDecisionReason reason = CognitiveDecisionReason::NONE;
    char intentId[37] = {0};
};

/**
 * @brief Creates a NO_ACTION decision.
 * @param reason The semantic reason no action is being taken.
 * @param authoritativeIntentId The authoritative intent ID for this decision, ensuring traceability.
 */
CognitiveDecision makeNoAction(
    CognitiveDecisionReason reason,
    const char* authoritativeIntentId
);

/**
 * @brief Creates an INTENT decision from a parsed intent.
 * @param intent The underlying intent to execute.
 * @param authoritativeIntentId The authoritative intent ID, which explicitly overwrites any ID on the incoming intent.
 */
CognitiveDecision makeIntent(
    const RobotIntent& intent,
    const char* authoritativeIntentId
);
