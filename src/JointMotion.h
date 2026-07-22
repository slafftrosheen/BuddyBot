#pragma once
#include <stdint.h>

enum class JointMotionState : uint8_t {
  IDLE,
  MOVING,
  COMPLETE,
  CANCELLED,
  UNAVAILABLE
};

enum class JointEasing : uint8_t {
  LINEAR,
  EASE_IN_OUT,
  EASE_OUT
};

struct JointMotionRequest {
  int16_t targetDeg;
  uint16_t durationMs;
  JointEasing easing;
};

class JointMotion {
public:
  void begin(int16_t initialDeg);
  bool start(int16_t fromDeg, int16_t toDeg, uint16_t durationMs, JointEasing easing);
  void update(uint32_t nowMs);
  void cancel();
  
  bool active() const;
  bool complete() const;
  JointMotionState state() const;
  
  int16_t currentTarget() const;
  int16_t currentValue() const;

private:
  int16_t _from = 90;
  int16_t _to = 90;
  int16_t _current = 90;
  uint32_t _startedMs = 0;
  uint16_t _durationMs = 0;
  JointEasing _easing = JointEasing::LINEAR;
  JointMotionState _state = JointMotionState::IDLE;
};
