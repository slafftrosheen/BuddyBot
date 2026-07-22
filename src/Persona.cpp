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
    "Safety stop!",
    FaceStyle::ROUND_SOFT,
    IdleStyle::CURIOUS,
    VoiceStyle::BRIGHT,
    34, 40, 6,
    3000, 6000,
    2000, 4000,
    true, true, false, true,
    CLR_PINK_NOVA, CLR_PINK_NOVA, CLR_PURPLE, CLR_CYAN,
    { 45, 300, 45, 135, 600, 300, true }
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
    "Safety stop!",
    FaceStyle::VISOR_TECH,
    IdleStyle::GENTLE,
    VoiceStyle::DEEP,
    44, 20, 4,
    4000, 8000,
    3000, 6000,
    false, false, true, false,
    CLR_BLUE_ORBIT, CLR_BLUE_ORBIT, CLR_YELLOW, CLR_CYAN,
    { 20, 600, 60, 120, 900, 800, false }
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
    "Safety stop!",
    FaceStyle::PIXEL_CUTE,
    IdleStyle::BOUNCY,
    VoiceStyle::NEUTRAL,
    30, 30, 8,
    2500, 5000,
    2000, 5000,
    false, true, false, true,
    CLR_GREEN, CLR_GREEN, CLR_CYAN, CLR_WHITE,
    { 60, 200, 30, 150, 400, 200, true }
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
    "Stopping.",
    FaceStyle::CUSTOM,
    IdleStyle::CUSTOM,
    VoiceStyle::CUSTOM,
    36, 36, 6,
    3000, 6000,
    2000, 5000,
    false, true, false, false,
    CLR_CYAN, CLR_PURPLE, CLR_CYAN, CLR_WHITE,
    { 30, 400, 45, 135, 500, 400, false }
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
    case Mood::CALM:     return CLR_CYAN;
    case Mood::PROUD:    return CLR_YELLOW;
    case Mood::SHY:      return CLR_PINK_NOVA;
    case Mood::SAD:      return CLR_BLUE_ORBIT;
    case Mood::CONFUSED: return CLR_PURPLE;
    case Mood::THINKING: return CLR_CYAN;
    case Mood::LISTENING:return CLR_GREEN;
    default:             return current().colors.accent;
  }
}

const char* moodName(Mood mood) {
  switch (mood) {
    case Mood::IDLE: return "IDLE";
    case Mood::HAPPY: return "HAPPY";
    case Mood::CURIOUS: return "CURIOUS";
    case Mood::SLEEPY: return "SLEEPY";
    case Mood::EXCITED: return "EXCITED";
    case Mood::ALERT: return "ALERT";
    case Mood::CALM: return "CALM";
    case Mood::PROUD: return "PROUD";
    case Mood::SHY: return "SHY";
    case Mood::SAD: return "SAD";
    case Mood::CONFUSED: return "CONFUSED";
    case Mood::THINKING: return "THINKING";
    case Mood::LISTENING: return "LISTENING";
    default: return "UNKNOWN";
  }
}

const char* expressionName(ExpressionId expression) {
  switch (expression) {
    case ExpressionId::NONE: return "NONE";
    case ExpressionId::BLINK: return "BLINK";
    case ExpressionId::DOUBLE_BLINK: return "DOUBLE_BLINK";
    case ExpressionId::WINK_LEFT: return "WINK_LEFT";
    case ExpressionId::WINK_RIGHT: return "WINK_RIGHT";
    case ExpressionId::LOOK_LEFT: return "LOOK_LEFT";
    case ExpressionId::LOOK_RIGHT: return "LOOK_RIGHT";
    case ExpressionId::LOOK_UP: return "LOOK_UP";
    case ExpressionId::LOOK_DOWN: return "LOOK_DOWN";
    case ExpressionId::SURPRISED: return "SURPRISED";
    case ExpressionId::CONFUSED: return "CONFUSED";
    case ExpressionId::WORRIED: return "WORRIED";
    case ExpressionId::SCARED: return "SCARED";
    case ExpressionId::GIGGLE: return "GIGGLE";
    case ExpressionId::LOVE: return "LOVE";
    case ExpressionId::PROUD: return "PROUD";
    case ExpressionId::SLEEP_YAWN: return "SLEEP_YAWN";
    case ExpressionId::LISTEN: return "LISTEN";
    case ExpressionId::THINK: return "THINK";
    case ExpressionId::CONNECTION_OK: return "CONNECTION_OK";
    case ExpressionId::CONNECTION_LOST: return "CONNECTION_LOST";
    case ExpressionId::OBSTACLE: return "OBSTACLE";
    case ExpressionId::LOW_BATTERY: return "LOW_BATTERY";
    case ExpressionId::COMMAND_REJECTED: return "COMMAND_REJECTED";
    default: return "UNKNOWN";
  }
}

static const ExpressionSpec EXPRESSION_SPECS[] = {
  { ExpressionId::NONE, 0, true, true, false, false, false },
  { ExpressionId::BLINK, 150, true, true, false, false, false },
  { ExpressionId::DOUBLE_BLINK, 400, true, true, false, false, false },
  { ExpressionId::WINK_LEFT, 200, false, true, false, false, false },
  { ExpressionId::WINK_RIGHT, 200, false, true, false, false, false },
  { ExpressionId::LOOK_LEFT, 900, true, true, false, false, true },
  { ExpressionId::LOOK_RIGHT, 900, true, true, false, false, true },
  { ExpressionId::LOOK_UP, 900, true, true, false, false, true },
  { ExpressionId::LOOK_DOWN, 900, true, true, false, false, true },
  { ExpressionId::SURPRISED, 1500, false, true, true, true, true },
  { ExpressionId::CONFUSED, 1800, true, true, true, true, true },
  { ExpressionId::WORRIED, 1800, true, true, true, true, true },
  { ExpressionId::SCARED, 2500, false, false, true, true, true },
  { ExpressionId::GIGGLE, 1500, false, true, true, true, true },
  { ExpressionId::LOVE, 2000, true, true, true, true, true },
  { ExpressionId::PROUD, 2000, true, true, true, true, true },
  { ExpressionId::SLEEP_YAWN, 3000, false, true, true, true, true },
  { ExpressionId::LISTEN, 3000, true, true, true, true, true },
  { ExpressionId::THINK, 3000, true, true, true, true, true },
  { ExpressionId::CONNECTION_OK, 1500, true, true, false, false, false },
  { ExpressionId::CONNECTION_LOST, 2500, true, false, true, true, true },
  { ExpressionId::OBSTACLE, 2000, true, false, true, true, true },
  { ExpressionId::LOW_BATTERY, 3000, true, false, true, true, true },
  { ExpressionId::COMMAND_REJECTED, 1500, true, false, true, true, true }
};

const ExpressionSpec& expressionSpec(ExpressionId id) {
  if (id >= ExpressionId::COUNT) return EXPRESSION_SPECS[0];
  return EXPRESSION_SPECS[uint8_t(id)];
}