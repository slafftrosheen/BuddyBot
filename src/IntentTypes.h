#pragma once

#include <stdint.h>
#include "Types.h"
#include "ControlTypes.h"
#include "RuntimeCapabilities.h"

enum class IntentKind : uint8_t {
    NONE = 0,
    MOVE,
    STOP,
    ARM,
    DISARM,
    ACTION,
    SET_MOOD,
    NEXT_PERSONA,
    ACCESSORY
};

struct RobotIntent {
    IntentKind kind = IntentKind::NONE;
    DriveMode driveMode = DriveMode::STOPPED;
    uint16_t durationMs = 0;
    ActionId action = ActionId::NONE;
    Mood mood = Mood::IDLE;
    uint8_t accessoryIndex = 0;
    bool accessoryActive = false;
    uint32_t correlationId = 0;
};

enum class IntentResolution : uint8_t {
    ACCEPTED = 0,
    INVALID_INTENT,
    CAPABILITY_UNAVAILABLE,
    NOT_PERMITTED,
    INVALID_PARAMETER
};

struct IntentResult {
    IntentResolution result;
    RobotCommand command;
    CapabilityReason reason;
    uint32_t correlationId;
};
