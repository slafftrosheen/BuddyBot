#pragma once
#include <Arduino.h>

enum class Mood : uint8_t {
  IDLE,
  HAPPY,
  CURIOUS,
  SLEEPY,
  EXCITED,
  ALERT,
  COUNT
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
};
