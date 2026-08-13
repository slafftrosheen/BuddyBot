#include "IntentResolver.h"

IntentResult IntentResolver::resolve(
    const RobotIntent& intent,
    const RuntimeCapabilities& capabilities
) const {
    IntentResult res;
    res.result = IntentResolution::INVALID_INTENT;
    res.reason = CapabilityReason::NONE;
    strncpy(res.intentId, intent.intentId, sizeof(res.intentId) - 1);
    res.intentId[sizeof(res.intentId) - 1] = '\0';
    res.result = IntentResolution::ACCEPTED;

    strncpy(res.command.intentId, intent.intentId, sizeof(res.command.intentId) - 1);
    res.command.intentId[sizeof(res.command.intentId) - 1] = '\0';
    res.command.kind = CommandKind::NONE;
    res.command.source = ControlSource::HALO;

    switch (intent.kind) {
        case IntentKind::NONE:
            res.result = IntentResolution::INVALID_INTENT;
            break;

        case IntentKind::MOVE: {
            if (!capabilities.drive.capable) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = CapabilityReason::NOT_IMPLEMENTED;
                break;
            }
            if (!capabilities.drive.available) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = capabilities.drive.reason;
                break;
            }
            if (!capabilities.drive.permitted) {
                res.result = IntentResolution::NOT_PERMITTED;
                res.reason = capabilities.drive.reason;
                break;
            }
            if (intent.driveMode == DriveMode::STOPPED || intent.durationMs == 0) {
                res.result = IntentResolution::INVALID_PARAMETER;
                res.reason = CapabilityReason::NONE;
                break;
            }
            res.result = IntentResolution::ACCEPTED;
            res.command.kind = CommandKind::MOVE;
            res.command.driveMode = intent.driveMode;
            res.command.durationMs = intent.durationMs;
            break;
        }

        case IntentKind::STOP: {
            res.result = IntentResolution::ACCEPTED;
            res.command.kind = CommandKind::STOP;
            break;
        }

        case IntentKind::ARM: {
            if (!capabilities.drive.capable) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = CapabilityReason::NOT_IMPLEMENTED;
                break;
            }
            if (!capabilities.drive.available) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = capabilities.drive.reason;
                break;
            }
            if (!capabilities.drive.permitted) {
                res.result = IntentResolution::NOT_PERMITTED;
                res.reason = capabilities.drive.reason;
                break;
            }
            res.result = IntentResolution::ACCEPTED;
            res.command.kind = CommandKind::ARM;
            break;
        }

        case IntentKind::DISARM: {
            res.result = IntentResolution::ACCEPTED;
            res.command.kind = CommandKind::DISARM;
            break;
        }

        case IntentKind::ACTION: {
            if (!capabilities.actions.capable) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = CapabilityReason::NOT_IMPLEMENTED;
                break;
            }
            if (!capabilities.actions.available) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = capabilities.actions.reason;
                break;
            }
            if (!capabilities.actions.permitted) {
                res.result = IntentResolution::NOT_PERMITTED;
                res.reason = capabilities.actions.reason;
                break;
            }
            if (intent.action == ActionId::NONE || intent.action >= ActionId::COUNT) {
                res.result = IntentResolution::INVALID_PARAMETER;
                res.reason = CapabilityReason::NONE;
                break;
            }
            res.result = IntentResolution::ACCEPTED;
            res.command.kind = CommandKind::ACTION;
            res.command.action = intent.action;
            break;
        }

        case IntentKind::SET_MOOD: {
            if (!capabilities.actions.capable) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = CapabilityReason::NOT_IMPLEMENTED;
                break;
            }
            if (!capabilities.actions.available) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = capabilities.actions.reason;
                break;
            }
            if (!capabilities.actions.permitted) {
                res.result = IntentResolution::NOT_PERMITTED;
                res.reason = capabilities.actions.reason;
                break;
            }
            if (intent.mood >= Mood::COUNT) {
                res.result = IntentResolution::INVALID_PARAMETER;
                res.reason = CapabilityReason::NONE;
                break;
            }
            res.result = IntentResolution::ACCEPTED;
            res.command.kind = CommandKind::SET_MOOD;
            res.command.mood = intent.mood;
            break;
        }

        case IntentKind::NEXT_PERSONA: {
            if (!capabilities.actions.capable) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = CapabilityReason::NOT_IMPLEMENTED;
                break;
            }
            if (!capabilities.actions.available) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = capabilities.actions.reason;
                break;
            }
            if (!capabilities.actions.permitted) {
                res.result = IntentResolution::NOT_PERMITTED;
                res.reason = capabilities.actions.reason;
                break;
            }
            res.result = IntentResolution::ACCEPTED;
            res.command.kind = CommandKind::NEXT_PERSONA;
            break;
        }

        case IntentKind::ACCESSORY: {
            if (!capabilities.manipulators.capable) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = CapabilityReason::NOT_IMPLEMENTED;
                break;
            }
            if (!capabilities.manipulators.available) {
                res.result = IntentResolution::CAPABILITY_UNAVAILABLE;
                res.reason = capabilities.manipulators.reason;
                break;
            }
            if (!capabilities.manipulators.permitted) {
                res.result = IntentResolution::NOT_PERMITTED;
                res.reason = capabilities.manipulators.reason;
                break;
            }
            res.result = IntentResolution::ACCEPTED;
            res.command.kind = CommandKind::ACCESSORY;
            res.command.index = intent.accessoryIndex;
            res.command.flag = intent.accessoryActive;
            break;
        }
    }

    return res;
}
