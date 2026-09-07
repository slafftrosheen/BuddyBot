#include "RuleReasoningProvider.h"
#include "Config.h"

CognitiveDecision RuleReasoningProvider::reason(const CognitiveContext& context) {
    // 1. ESTOP
    if (context.snapshot.safety.estopped) {
        return makeNoAction(CognitiveDecisionReason::SAFETY_BLOCKED, context.intentId);
    }

    // 2. SAFETY FAULT
    if (context.snapshot.safety.faulted) {
        return makeNoAction(CognitiveDecisionReason::SAFETY_BLOCKED, context.intentId);
    }

    // 3. BOOT INCOMPLETE
    if (!context.snapshot.safety.bootComplete) {
        return makeNoAction(CognitiveDecisionReason::OBSERVATION_ONLY, context.intentId);
    }

    // 4. DRIVE UNAVAILABLE
    if (!context.capabilities.drive.capable || !context.capabilities.drive.available) {
        return makeNoAction(CognitiveDecisionReason::CAPABILITY_UNAVAILABLE, context.intentId);
    }

    // 5. AUTONOMY DISABLED
    if (!context.snapshot.behavior.autonomyEnabled) {
        return makeNoAction(CognitiveDecisionReason::OBSERVATION_ONLY, context.intentId);
    }

    // 6. AUTONOMY NOT PERMITTED
    if (!context.snapshot.safety.autonomyMotionAllowed) {
        return makeNoAction(CognitiveDecisionReason::SAFETY_BLOCKED, context.intentId);
    }

    // 7. DRIVE PERMITTED — choose action based on range sensor data
    if (context.capabilities.drive.permitted) {
        // If range sensor is unavailable or invalid, don't move blindly
        if (!context.snapshot.range.available || !context.snapshot.range.valid) {
            return makeNoAction(CognitiveDecisionReason::OBSERVATION_ONLY, context.intentId);
        }

        const uint16_t rangeMm = context.snapshot.range.distanceMm;

        RobotIntent intent;
        strncpy(intent.intentId, context.intentId, sizeof(intent.intentId) - 1);
        intent.intentId[sizeof(intent.intentId) - 1] = '\0';

        if (rangeMm < OBSTACLE_EMERGENCY_STOP_MM) {
            // Too close — reverse away
            intent.kind = IntentKind::MOVE;
            intent.driveMode = DriveMode::REVERSE;
            intent.durationMs = AUTONOMY_REVERSE_MS;
        } else if (rangeMm < OBSTACLE_CAUTION_MM) {
            // Caution zone — turn to find clearance
            intent.kind = IntentKind::MOVE;
            intent.driveMode = DriveMode::TURN_RIGHT;
            intent.durationMs = AUTONOMY_TURN_MS;
        } else {
            // Clear path — proceed forward
            intent.kind = IntentKind::MOVE;
            intent.driveMode = DriveMode::FORWARD;
            intent.durationMs = 250;
        }

        return makeIntent(intent, context.intentId);
    }

    // Default catch-all
    return makeNoAction(CognitiveDecisionReason::NO_ACTION_REQUIRED, context.intentId);
}
