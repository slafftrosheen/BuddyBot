#include "PersonaParticles.h"

void PersonaParticlePool::begin() {
  clear();
  _enabled = ENABLE_PERSONA_PARTICLES;
  _randomSeed = 9999;
}

uint32_t PersonaParticlePool::randomRange(uint32_t minVal, uint32_t maxVal) {
  if (minVal >= maxVal) return minVal;
  _randomSeed = (_randomSeed * 1103515245 + 12345) & 0x7fffffff;
  return minVal + (_randomSeed % (maxVal - minVal + 1));
}

void PersonaParticlePool::clear() {
  for (uint8_t i = 0; i < MAX_PERSONA_PARTICLES; i++) {
    _particles[i].active = false;
  }
}

void PersonaParticlePool::setEnabled(bool enabled) {
  _enabled = enabled && ENABLE_PERSONA_PARTICLES;
  if (!_enabled) {
    clear();
  }
}

void PersonaParticlePool::spawn(int16_t x, int16_t y, ParticleKind kind, uint16_t color) {
  if (!_enabled) return;

  for (uint8_t i = 0; i < MAX_PERSONA_PARTICLES; i++) {
    if (!_particles[i].active) {
      _particles[i].active = true;
      _particles[i].x = x;
      _particles[i].y = y;
      _particles[i].kind = kind;
      _particles[i].color = color;
      
      if (kind == ParticleKind::SPARK) {
        _particles[i].vx = (int8_t)randomRange(0, 4) - 2;
        _particles[i].vy = (int8_t)randomRange(1, 4) * -1;
        _particles[i].size = 2;
        _particles[i].maxLife = randomRange(10, 20);
      } else if (kind == ParticleKind::HEART) {
        _particles[i].vx = 0;
        _particles[i].vy = -2;
        _particles[i].size = 4;
        _particles[i].maxLife = 25;
      } else if (kind == ParticleKind::STAR || kind == ParticleKind::CONFETTI) {
        _particles[i].vx = (int8_t)randomRange(0, 8) - 4;
        _particles[i].vy = (int8_t)randomRange(2, 6) * -1;
        _particles[i].size = 3;
        _particles[i].maxLife = 30;
      } else if (kind == ParticleKind::SLEEP_Z) {
        _particles[i].vx = 1;
        _particles[i].vy = -1;
        _particles[i].size = randomRange(2, 4);
        _particles[i].maxLife = 30;
      } else if (kind == ParticleKind::SIGNAL) {
        _particles[i].vx = (int8_t)randomRange(0, 4) - 2;
        _particles[i].vy = (int8_t)randomRange(2, 5) * -1;
        _particles[i].size = 2;
        _particles[i].maxLife = 15;
      } else if (kind == ParticleKind::TEAR || kind == ParticleKind::SWEAT) {
        _particles[i].vx = 0;
        _particles[i].vy = 2;
        _particles[i].size = 3;
        _particles[i].maxLife = 20;
      }

      _particles[i].life = _particles[i].maxLife;
      break;
    }
  }
}

void PersonaParticlePool::update() {
  if (!_enabled) return;

  for (uint8_t i = 0; i < MAX_PERSONA_PARTICLES; i++) {
    if (_particles[i].active) {
      if (_particles[i].life > 0) {
        _particles[i].life--;
        _particles[i].x += _particles[i].vx;
        _particles[i].y += _particles[i].vy;

        if (_particles[i].kind == ParticleKind::CONFETTI) {
          _particles[i].vy += 1; // Gravity
        }
        
        // Bounds check
        if (_particles[i].x < 0 || _particles[i].x > SCREEN_W || _particles[i].y < 0 || _particles[i].y > SCREEN_H) {
           _particles[i].active = false;
        }
      } else {
        _particles[i].active = false;
      }
    }
  }
}

void PersonaParticlePool::draw(M5Canvas& canvas) const {
  if (!_enabled) return;

  for (uint8_t i = 0; i < MAX_PERSONA_PARTICLES; i++) {
    if (_particles[i].active) {
      if (_particles[i].kind == ParticleKind::SPARK || _particles[i].kind == ParticleKind::SIGNAL || _particles[i].kind == ParticleKind::CONFETTI) {
        canvas.fillRect(_particles[i].x, _particles[i].y, _particles[i].size, _particles[i].size, _particles[i].color);
      } else if (_particles[i].kind == ParticleKind::HEART) {
        // Simple heart shape
        canvas.fillCircle(_particles[i].x - 1, _particles[i].y, 2, _particles[i].color);
        canvas.fillCircle(_particles[i].x + 1, _particles[i].y, 2, _particles[i].color);
        canvas.fillTriangle(_particles[i].x - 3, _particles[i].y + 1, _particles[i].x + 3, _particles[i].y + 1, _particles[i].x, _particles[i].y + 4, _particles[i].color);
      } else if (_particles[i].kind == ParticleKind::STAR) {
        // Simple cross for a star
        canvas.fillRect(_particles[i].x - 1, _particles[i].y - 3, 3, 7, _particles[i].color);
        canvas.fillRect(_particles[i].x - 3, _particles[i].y - 1, 7, 3, _particles[i].color);
      } else if (_particles[i].kind == ParticleKind::SLEEP_Z) {
        // Letter Z
        canvas.drawLine(_particles[i].x, _particles[i].y, _particles[i].x + 4, _particles[i].y, _particles[i].color);
        canvas.drawLine(_particles[i].x + 4, _particles[i].y, _particles[i].x, _particles[i].y + 4, _particles[i].color);
        canvas.drawLine(_particles[i].x, _particles[i].y + 4, _particles[i].x + 4, _particles[i].y + 4, _particles[i].color);
      } else if (_particles[i].kind == ParticleKind::TEAR || _particles[i].kind == ParticleKind::SWEAT) {
        // Droplet shape
        canvas.fillCircle(_particles[i].x, _particles[i].y + 2, 2, _particles[i].color);
        canvas.fillTriangle(_particles[i].x - 2, _particles[i].y + 2, _particles[i].x + 2, _particles[i].y + 2, _particles[i].x, _particles[i].y - 2, _particles[i].color);
      }
    }
  }
}
