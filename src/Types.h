#pragma once
#include <Arduino.h>

enum class Mood : uint8_t {
  IDLE,
  HAPPY,
  CURIOUS,
  SLEEPY,
  EXCITED,
  ALERT,
  CALM,
  PROUD,
  SHY,
  SAD,
  CONFUSED,
  THINKING,
  LISTENING,
  COUNT
};

enum class ExpressionId : uint8_t {
  NONE,
  BLINK,
  DOUBLE_BLINK,
  WINK_LEFT,
  WINK_RIGHT,
  LOOK_LEFT,
  LOOK_RIGHT,
  LOOK_UP,
  LOOK_DOWN,
  SURPRISED,
  CONFUSED,
  WORRIED,
  SCARED,
  GIGGLE,
  LOVE,
  PROUD,
  SLEEP_YAWN,
  LISTEN,
  THINK,
  CONNECTION_OK,
  CONNECTION_LOST,
  OBSTACLE,
  LOW_BATTERY,
  COMMAND_REJECTED,
  COUNT
};

enum class AttentionTarget : uint8_t {
  CENTER,
  LEFT,
  RIGHT,
  UP,
  DOWN,
  RANDOM,
  CONTROLLER,
  OBSTACLE,
  SOUND
};

struct ExpressionSpec {
  ExpressionId id;
  uint16_t defaultDurationMs;
  bool allowBlink;
  bool interruptible;
  bool affectsMouth;
  bool affectsBrows;
  bool affectsPupils;
};

enum class PersonaId : uint8_t {
  NOVA,
  ORBIT,
  PIXEL,
  CUSTOM,
  COUNT
};

enum class Accessory : uint8_t {
  NONE,
  STAR_BADGE,
  VISOR,
  BOW,
  ANTENNA,
  EARS,
  HAT
};

enum class DriveMode : uint8_t {
  STOPPED,
  FORWARD,
  REVERSE,
  TURN_LEFT,
  TURN_RIGHT
};

enum class ActionId : uint8_t {
  NONE,
  WAVE,
  LOOK_LEFT,
  LOOK_RIGHT,
  CELEBRATE,
  DANCE,
  GREET,
  SLEEP,
  COUNT
};

struct ColorTheme {
  uint16_t accent;
  uint16_t secondary;
  uint16_t eye;
  uint16_t background;
};

enum class FaceStyle : uint8_t {
  ROUND_SOFT,
  VISOR_TECH,
  PIXEL_CUTE,
  CUSTOM
};

enum class IdleStyle : uint8_t {
  GENTLE,
  CURIOUS,
  BOUNCY,
  SLEEPY,
  CUSTOM
};

enum class VoiceStyle : uint8_t {
  BRIGHT,
  DEEP,
  NEUTRAL,
  CUSTOM
};

struct PersonaGestureProfile {
  int16_t waveAmplitudeDeg;
  uint16_t waveStepMs;
  int16_t lookLeftDeg;
  int16_t lookRightDeg;
  uint16_t lookSpeedMs;
  uint16_t danceStepMs;
  bool useAccessoryDuringCelebrate;
};

struct PersonaProfile {
  PersonaId id;
  const char* name;
  const char* subtitle;
  ColorTheme colors;
  float voicePitch;
  Accessory headAccessory;
  Accessory sideAccessory;
  const char* idleText;
  const char* happyText;
  const char* curiousText;
  const char* sleepyText;
  const char* excitedText;
  const char* alertText;
  FaceStyle faceStyle;
  IdleStyle idleStyle;
  VoiceStyle voiceStyle;
  uint8_t eyeWidth;
  uint8_t eyeHeight;
  uint8_t pupilRadius;
  uint16_t blinkMinMs;
  uint16_t blinkMaxMs;
  uint16_t idleLookMinMs;
  uint16_t idleLookMaxMs;
  bool showFreckles;
  bool showEyeHighlights;
  bool showScanLines;
  bool showAntennaPulse;
  uint16_t accentGlowColor;
  uint16_t cheekColor;
  uint16_t panelColor;
  uint16_t textColor;
  PersonaGestureProfile gestures;
};
