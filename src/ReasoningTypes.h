#pragma once
#include <stdint.h>

enum class ReasoningResult : uint8_t {
    SUCCESS = 0,
    NO_DECISION,
    PROVIDER_UNAVAILABLE,
    PROVIDER_ERROR,
    INVALID_DECISION
};
