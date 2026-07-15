#include "Persona.h"

// All custom colour constants begin with CLR_.
// This prevents conflicts with M5GFX's built-in BLACK, RED, PINK, etc.

static constexpr uint16_t CLR_BG_DARK    = 0x0092;
static constexpr uint16_t CLR_PINK_NOVA  = 0xF857;
static constexpr uint16_t CLR_BLUE_ORBIT = 0x3D7F;
static constexpr uint16_t CLR_CYAN       = 0x2DFF;
static constexpr uint16_t CLR_PURPLE     = 0xA99B;
static constexpr uint16_t CLR_GREEN      = 0x4E79;
static constexpr uint16_t CLR_YELLOW     = 0xFEA0;
static constexpr uint16_t CLR_RED_ALERT  = 0xF986;
static constexpr uint16_t CLR_WHITE      = 0xFFFF;

static const PersonaProfile PROFILES[] = {
  {
    PersonaId::NOVA,
    "NOVA",
    "explorer mode",
    {CLR_PINK_NOVA, CLR_PURPLE, CLR_CYAN, CLR_BG_DARK},
    1.18f,
    Accessory::ANTENNA,
    Accessory::STAR_BADGE,
    "Hi, builder!",
    "That was fun!",
    "What is that?",
    "Energy saving...",
    "Adventure time!",
    "Safety stop!"
  },

  {
    PersonaId::ORBIT,
    "ORBIT",
    "rover mode",
    {CLR_BLUE_ORBIT, CLR_YELLOW, CLR_CYAN, CLR_BG_DARK},
    0.88f,
    Accessory::VISOR,
    Accessory::NONE,
    "Systems ready!",
    "Mission success!",
    "Scanning...",
    "Low activity...",
    "Lets roll!",
    "Safety stop!"
  },

  {
    PersonaId::PIXEL,
    "PIXEL",
    "creative mode",
    {CLR_GREEN, CLR_CYAN, CLR_WHITE, CLR_BG_DARK},
    1.00f,
    Accessory::ANTENNA,
    Accessory::NONE,
    "Hello, builder!",
    "Nice work!",
    "Interesting...",
    "Quiet mode...",
    "Lets create!",
    "Safety stop!"
  },

  {
    PersonaId::CUSTOM,
    "CUSTOM",
    "your creation",
    {CLR_CYAN, CLR_PURPLE, CLR_WHITE, CLR_BG_DARK},
    1.00f,
    Accessory::NONE,
    Accessory::NONE,
    "Hello!",
    "Amazing!",
    "Tell me more.",
    "Resting...",
    "Lets go!",
    "Stopping."
  }
};

void PersonaManager::begin() {
  _id = PersonaId::PIXEL;
}

void PersonaManager::next() {
  uint8_t nextId = (uint8_t(_id) + 1) % uint8_t(PersonaId::COUNT);
  _id = PersonaId(nextId);
}

void PersonaManager::set(PersonaId id) {
  _id = id;
}

PersonaId PersonaManager::id() const {
  return _id;
}

const PersonaProfile& PersonaManager::current() const {
  return PROFILES[uint8_t(_id)];
}

const char* PersonaManager::messageFor(Mood mood) const {
  const PersonaProfile& p = current();

  switch (mood) {
    case Mood::HAPPY:    return p.happyText;
    case Mood::CURIOUS:  return p.curiousText;
    case Mood::SLEEPY:   return p.sleepyText;
    case Mood::EXCITED:  return p.excitedText;
    case Mood::ALERT:    return p.alertText;
    default:             return p.idleText;
  }
}

uint16_t PersonaManager::moodColor(Mood mood) const {
  switch (mood) {
    case Mood::HAPPY:    return CLR_GREEN;
    case Mood::CURIOUS:  return CLR_CYAN;
    case Mood::SLEEPY:   return CLR_PURPLE;
    case Mood::EXCITED:  return CLR_YELLOW;
    case Mood::ALERT:    return CLR_RED_ALERT;
    default:             return current().colors.accent;
  }
}