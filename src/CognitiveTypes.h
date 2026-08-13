#pragma once
#include <stdint.h>

/*
 * ARCHITECTURAL INVARIANT:
 * The cognitive layer produces Intent only.
 * IntentResolver determines whether that Intent can become a RobotCommand.
 * SafetySupervisor remains the final execution authority.
 */

enum class CognitiveDecisionKind : uint8_t {
    NONE = 0,
    INTENT,
    NO_ACTION
};

enum class CognitiveDecisionReason : uint8_t {
    NONE = 0,
    NO_ACTION_REQUIRED,
    OBSERVATION_ONLY,
    CAPABILITY_UNAVAILABLE,
    SAFETY_BLOCKED,
    INVALID_INTENT,
    INTERNAL_ERROR
};
