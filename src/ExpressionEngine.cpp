#include "ExpressionEngine.h"

void ExpressionEngine::begin() {
  _baseMood = Mood::IDLE;
  _expression = ExpressionId::NONE;
  _attention = AttentionTarget::CENTER;
  _expressionStartedMs = 0;
  _expressionEndsMs = 0;
  _lastInteractionMs = 0;
  _nextIdleActionMs = 0;
  _nextBlinkMs = 0;
  _blinkStartMs = 0;
  _blinkPhase = 0;
  _isDoubleBlink = false;
  _isMoving = false;
  _randomSeed = 12345;
  clear();
}

void ExpressionEngine::setRandomSeed(uint32_t seed) {
  _randomSeed = seed;
}

uint32_t ExpressionEngine::randomRange(uint32_t minVal, uint32_t maxVal) {
  if (minVal >= maxVal) return minVal;
  // A simple deterministic LCG for animations
  _randomSeed = (_randomSeed * 1103515245 + 12345) & 0x7fffffff;
  return minVal + (_randomSeed % (maxVal - minVal + 1));
}

void ExpressionEngine::setBaseMood(Mood mood) {
  _baseMood = mood;
}

Mood ExpressionEngine::baseMood() const {
  return _baseMood;
}

bool ExpressionEngine::play(ExpressionId expression, uint16_t durationMs) {
  if (expression == ExpressionId::NONE) {
    clear();
    return true;
  }

  const ExpressionSpec& newSpec = expressionSpec(expression);
  const ExpressionSpec& curSpec = expressionSpec(_expression);

  // Priority overriding
  bool isUrgentNew = (expression == ExpressionId::OBSTACLE || 
                      expression == ExpressionId::SCARED || 
                      expression == ExpressionId::CONNECTION_LOST || 
                      expression == ExpressionId::LOW_BATTERY || 
                      expression == ExpressionId::COMMAND_REJECTED);
  
  if (_expression != ExpressionId::NONE) {
    if (!curSpec.interruptible && !isUrgentNew) {
      return false; // Cannot interrupt
    }
  }

  _expression = expression;
  _expressionStartedMs = millis();
  
  if (durationMs > 0) {
    _expressionEndsMs = _expressionStartedMs + durationMs;
  } else {
    _expressionEndsMs = _expressionStartedMs + newSpec.defaultDurationMs;
  }
  
  return true;
}

void ExpressionEngine::clear() {
  _expression = ExpressionId::NONE;
  _expressionEndsMs = 0;
}

bool ExpressionEngine::active() const {
  return _expression != ExpressionId::NONE;
}

ExpressionId ExpressionEngine::current() const {
  return _expression;
}

void ExpressionEngine::setAttention(AttentionTarget target) {
  _attention = target;
}

AttentionTarget ExpressionEngine::attention() const {
  return _attention;
}

void ExpressionEngine::notifyUserInteraction() {
  _lastInteractionMs = millis();
}

void ExpressionEngine::notifyMotionStarted() {
  _isMoving = true;
}

void ExpressionEngine::notifyMotionStopped() {
  _isMoving = false;
}

void ExpressionEngine::notifyObstacle() {
  play(ExpressionId::OBSTACLE);
}

void ExpressionEngine::notifyWifiControllerConnected() {
  play(ExpressionId::CONNECTION_OK);
}

void ExpressionEngine::notifyWifiControllerDisconnected() {
  // Don't override OBSTACLE
  if (_expression != ExpressionId::OBSTACLE) {
    play(ExpressionId::CONNECTION_LOST);
  }
}

void ExpressionEngine::notifyCommandRejected() {
  static uint32_t lastRejectionMs = 0;
  if (millis() - lastRejectionMs > 2000) {
    play(ExpressionId::COMMAND_REJECTED);
    lastRejectionMs = millis();
  }
}

void ExpressionEngine::notifyLowBattery() {
  play(ExpressionId::LOW_BATTERY);
}

void ExpressionEngine::update(uint32_t nowMs, const PersonaManager& personaManager) {
  // Expiry check
  if (_expression != ExpressionId::NONE && _expressionEndsMs > 0 && nowMs >= _expressionEndsMs) {
    clear();
  }

  updateBlink(nowMs, personaManager);
  updateIdle(nowMs, personaManager);
  evaluateState(nowMs);
}

void ExpressionEngine::updateBlink(uint32_t nowMs, const PersonaManager& personaManager) {
  const PersonaProfile& profile = personaManager.current();
  
  const ExpressionSpec& curSpec = expressionSpec(_expression);
  if (!curSpec.allowBlink) {
    _blinkPhase = 0;
    return;
  }

  if (_blinkPhase == 0) {
    if (nowMs >= _nextBlinkMs) {
      _blinkPhase = 1; // start closing
      _blinkStartMs = nowMs;
      _isDoubleBlink = (randomRange(0, 10) > 7); // 20-30% chance of double blink
    }
  } else {
    uint32_t elapsed = nowMs - _blinkStartMs;
    uint32_t blinkDuration = _isDoubleBlink ? 300 : 150;
    
    if (elapsed > blinkDuration) {
      _blinkPhase = 0;
      _nextBlinkMs = nowMs + randomRange(profile.blinkMinMs, profile.blinkMaxMs);
    } else if (elapsed > blinkDuration / 2) {
      _blinkPhase = 2; // opening
    } else {
      _blinkPhase = 1; // closing
    }
  }
}

void ExpressionEngine::updateIdle(uint32_t nowMs, const PersonaManager& personaManager) {
  const PersonaProfile& profile = personaManager.current();
  
  if (_expression == ExpressionId::NONE) {
    if (nowMs >= _nextIdleActionMs) {
      // Pick a random idle glance or bounce depending on profile
      uint32_t r = randomRange(0, 100);
      
      if (profile.idleStyle == IdleStyle::SLEEPY || _baseMood == Mood::SLEEPY) {
         if (r < 30) play(ExpressionId::SLEEP_YAWN);
         else if (r < 60) _attention = AttentionTarget::DOWN;
         else _attention = AttentionTarget::CENTER;
      } else if (profile.idleStyle == IdleStyle::BOUNCY) {
         if (r < 20) play(ExpressionId::GIGGLE, 500);
         else if (r < 50) _attention = AttentionTarget::RANDOM;
         else _attention = AttentionTarget::CENTER;
      } else if (profile.idleStyle == IdleStyle::CURIOUS || _baseMood == Mood::CURIOUS) {
         if (r < 40) _attention = AttentionTarget::RANDOM;
         else if (r < 60) play(ExpressionId::THINK, 1000);
         else _attention = AttentionTarget::CENTER;
      } else {
         if (r < 30) _attention = AttentionTarget::RANDOM;
         else _attention = AttentionTarget::CENTER;
      }
      
      _nextIdleActionMs = nowMs + randomRange(profile.idleLookMinMs, profile.idleLookMaxMs);
    }
  }
  
  // Resolve random attention
  if (_attention == AttentionTarget::RANDOM) {
     uint32_t rr = randomRange(0, 4);
     if (rr == 0) _attention = AttentionTarget::LEFT;
     else if (rr == 1) _attention = AttentionTarget::RIGHT;
     else if (rr == 2) _attention = AttentionTarget::UP;
     else _attention = AttentionTarget::CENTER;
  }
}

void ExpressionEngine::evaluateState(uint32_t nowMs) {
  // Default neutral values
  _eyeOpenLeft = 1.0f;
  _eyeOpenRight = 1.0f;
  _pupilOffsetX = 0;
  _pupilOffsetY = 0;
  _browTiltLeft = 0;
  _browTiltRight = 0;
  _mouthOpenness = 0;
  _cheekIntensity = 0;
  _shakeAmount = 0;
  _bounceAmount = 0;

  // Apply Blink
  if (_blinkPhase == 1) {
    _eyeOpenLeft = 0.1f;
    _eyeOpenRight = 0.1f;
  } else if (_blinkPhase == 2) {
    _eyeOpenLeft = 0.5f;
    _eyeOpenRight = 0.5f;
  }

  // Base mood defaults
  if (_baseMood == Mood::HAPPY) {
    _browTiltLeft = -10;
    _browTiltRight = 10;
    _mouthOpenness = 10;
    _cheekIntensity = 200;
  } else if (_baseMood == Mood::SLEEPY) {
    if (_blinkPhase == 0) {
      _eyeOpenLeft = 0.4f;
      _eyeOpenRight = 0.4f;
    }
  } else if (_baseMood == Mood::ALERT) {
    _eyeOpenLeft = 1.2f;
    _eyeOpenRight = 1.2f;
    _browTiltLeft = 20;
    _browTiltRight = -20;
  } else if (_baseMood == Mood::SAD) {
    _browTiltLeft = 15;
    _browTiltRight = -15;
    _mouthOpenness = -5;
  }

  // Expression overrides
  if (_expression == ExpressionId::LOOK_LEFT) {
    _pupilOffsetX = -10;
  } else if (_expression == ExpressionId::LOOK_RIGHT) {
    _pupilOffsetX = 10;
  } else if (_expression == ExpressionId::LOOK_UP) {
    _pupilOffsetY = -10;
  } else if (_expression == ExpressionId::LOOK_DOWN) {
    _pupilOffsetY = 10;
  } else if (_expression == ExpressionId::SURPRISED) {
    _eyeOpenLeft = 1.5f;
    _eyeOpenRight = 1.5f;
    _mouthOpenness = 15;
    _browTiltLeft = -25;
    _browTiltRight = 25;
  } else if (_expression == ExpressionId::CONFUSED) {
    _browTiltLeft = -20;
    _browTiltRight = 0;
    _pupilOffsetX = 5;
    _pupilOffsetY = -5;
  } else if (_expression == ExpressionId::WORRIED) {
    _browTiltLeft = 20;
    _browTiltRight = -20;
    _eyeOpenLeft = 0.6f;
    _eyeOpenRight = 0.6f;
  } else if (_expression == ExpressionId::SCARED) {
    _eyeOpenLeft = 1.5f;
    _eyeOpenRight = 1.5f;
    _shakeAmount = 4;
  } else if (_expression == ExpressionId::GIGGLE) {
    _eyeOpenLeft = 0.2f;
    _eyeOpenRight = 0.2f;
    _cheekIntensity = 255;
    _bounceAmount = 2;
  } else if (_expression == ExpressionId::LOVE) {
    _cheekIntensity = 255;
  } else if (_expression == ExpressionId::PROUD) {
    _browTiltLeft = -15;
    _browTiltRight = 15;
    _mouthOpenness = 10;
    _pupilOffsetY = -5;
  } else if (_expression == ExpressionId::SLEEP_YAWN) {
    _eyeOpenLeft = 0.2f;
    _eyeOpenRight = 0.2f;
    _mouthOpenness = 20;
  } else if (_expression == ExpressionId::THINK) {
    _pupilOffsetX = -8;
    _pupilOffsetY = -8;
  } else if (_expression == ExpressionId::OBSTACLE) {
    _eyeOpenLeft = 1.2f;
    _eyeOpenRight = 1.2f;
    _pupilOffsetX = 0;
    _pupilOffsetY = 0;
    _browTiltLeft = 20;
    _browTiltRight = -20;
  } else if (_expression == ExpressionId::COMMAND_REJECTED) {
    _shakeAmount = 3;
    _browTiltLeft = 15;
    _browTiltRight = -15;
  } else if (_expression == ExpressionId::WINK_LEFT) {
    _eyeOpenLeft = 0.0f;
  } else if (_expression == ExpressionId::WINK_RIGHT) {
    _eyeOpenRight = 0.0f;
  }

  // Attention overrides (if not overridden by expression)
  const ExpressionSpec& curSpec = expressionSpec(_expression);
  if (!curSpec.affectsPupils) {
    if (_attention == AttentionTarget::LEFT) _pupilOffsetX = -8;
    else if (_attention == AttentionTarget::RIGHT) _pupilOffsetX = 8;
    else if (_attention == AttentionTarget::UP) _pupilOffsetY = -8;
    else if (_attention == AttentionTarget::DOWN) _pupilOffsetY = 8;
  }
}

float ExpressionEngine::eyeOpenLeft() const { return _eyeOpenLeft; }
float ExpressionEngine::eyeOpenRight() const { return _eyeOpenRight; }
int8_t ExpressionEngine::pupilOffsetX() const { return _pupilOffsetX; }
int8_t ExpressionEngine::pupilOffsetY() const { return _pupilOffsetY; }
int8_t ExpressionEngine::browTiltLeft() const { return _browTiltLeft; }
int8_t ExpressionEngine::browTiltRight() const { return _browTiltRight; }
int8_t ExpressionEngine::mouthOpenness() const { return _mouthOpenness; }
uint8_t ExpressionEngine::cheekIntensity() const { return _cheekIntensity; }
uint8_t ExpressionEngine::shakeAmount() const { return _shakeAmount; }
uint8_t ExpressionEngine::bounceAmount() const { return _bounceAmount; }

bool ExpressionEngine::showTear() const { return _baseMood == Mood::SAD; }
bool ExpressionEngine::showSweatDrop() const { return _expression == ExpressionId::WORRIED; }
bool ExpressionEngine::showHeart() const { return _expression == ExpressionId::LOVE; }
bool ExpressionEngine::showQuestionMark() const { return _expression == ExpressionId::CONFUSED; }
bool ExpressionEngine::showExclamationMark() const { return _expression == ExpressionId::SURPRISED; }
bool ExpressionEngine::showSleepBubble() const { return _expression == ExpressionId::SLEEP_YAWN || (_expression == ExpressionId::NONE && _baseMood == Mood::SLEEPY); }
bool ExpressionEngine::showListeningBars() const { return _expression == ExpressionId::LISTEN; }
bool ExpressionEngine::showThinkingDots() const { return _expression == ExpressionId::THINK; }
bool ExpressionEngine::showWarningSymbol() const { return _expression == ExpressionId::SCARED || _expression == ExpressionId::OBSTACLE; }
