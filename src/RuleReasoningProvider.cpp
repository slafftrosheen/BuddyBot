#include "RuleReasoningProvider.h"

CognitiveDecision RuleReasoningProvider::reason(const CognitiveContext& context) {
    // 1. ESTOP
    if (context.snapshot.safety.estopped) {
        return makeNoAction(CognitiveDecisionReason::SAFETY_BLOCKED, context.correlationId);
    }

    // 2. SAFETY FAULT
    if (context.snapshot.safety.faulted) {
        return makeNoAction(CognitiveDecisionReason::SAFETY_BLOCKED, context.correlationId);
    }

    // 3. BOOT INCOMPLETE
    if (!context.snapshot.safety.bootComplete) {
        return makeNoAction(CognitiveDecisionReason::OBSERVATION_ONLY, context.correlationId);
    }

    // 4. DRIVE UNAVAILABLE
    if (!context.capabilities.drive.capable || !context.capabilities.drive.available) {
        return makeNoAction(CognitiveDecisionReason::CAPABILITY_UNAVAILABLE, context.correlationId);
    }

    // 5. AUTONOMY DISABLED
    if (!context.snapshot.behavior.autonomyEnabled) {
        return makeNoAction(CognitiveDecisionReason::OBSERVATION_ONLY, context.correlationId);
    }

    // 6. AUTONOMY NOT PERMITTED
    if (!context.snapshot.safety.autonomyMotionAllowed) {
        return makeNoAction(CognitiveDecisionReason::SAFETY_BLOCKED, context.correlationId);
    }

    // 7. SAFE MOVE INTENT
    if (context.capabilities.drive.permitted) {
        RobotIntent intent;
        intent.kind = IntentKind::MOVE;
        intent.driveMode = DriveMode::FORWARD;
        intent.durationMs = 250;
        intent.correlationId = context.correlationId;

        return makeIntent(intent, context.correlationId);
    }

    // Default catch-all (should not be reached if permitted aligns with above)
    return makeNoAction(CognitiveDecisionReason::NO_ACTION_REQUIRED, context.correlationId);
}
