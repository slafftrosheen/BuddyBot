#pragma once
#include "Types.h"
#include "Config.h"
#include <M5Unified.h>

enum class ParticleKind : uint8_t {
  SPARK,
  HEART,
  STAR,
  SLEEP_Z,
  SIGNAL,
  TEAR,
  SWEAT,
  CONFETTI
};

struct PersonaParticle {
  int16_t x;
  int16_t y;
  int8_t vx;
  int8_t vy;
  uint16_t color;
  uint8_t size;
  uint8_t life;
  uint8_t maxLife;
  ParticleKind kind;
  bool active;
};

class PersonaParticlePool {
public:
  void begin();
  void update();
  void draw(M5Canvas& canvas) const;
  void spawn(int16_t x, int16_t y, ParticleKind kind, uint16_t color);
  void clear();
  void setEnabled(bool enabled);

private:
  PersonaParticle _particles[MAX_PERSONA_PARTICLES];
  bool _enabled = true;
  uint32_t _randomSeed = 9999;
  uint32_t randomRange(uint32_t minVal, uint32_t maxVal);
};
