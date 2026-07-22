#pragma once

#include "Types.h"
#include "Persona.h"

class ExpressionEngine {
public:
  void begin();
  void update(uint32_t nowMs, const PersonaManager& personaManager);

  void setBaseMood(Mood mood);
  Mood baseMood() const;

  bool play(ExpressionId expression, uint16_t durationMs = 0);
  void clear();
  bool active() const;
  ExpressionId current() const;

  void setAttention(AttentionTarget target);
  AttentionTarget attention() const;

  void notifyUserInteraction();
  void notifyMotionStarted();
  void notifyMotionStopped();
  void notifyObstacle();
  void notifyWifiControllerConnected();
  void notifyWifiControllerDisconnected();
  void notifyCommandRejected();
  void notifyLowBattery();

  float eyeOpenLeft() const;
  float eyeOpenRight() const;
  int8_t pupilOffsetX() const;
  int8_t pupilOffsetY() const;
  int8_t browTiltLeft() const;
  int8_t browTiltRight() const;
  int8_t mouthOpenness() const;
  uint8_t cheekIntensity() const;
  uint8_t shakeAmount() const;
  uint8_t bounceAmount() const;

  bool showTear() const;
  bool showSweatDrop() const;
  bool showHeart() const;
  bool showQuestionMark() const;
  bool showExclamationMark() const;
  bool showSleepBubble() const;
  bool showListeningBars() const;
  bool showThinkingDots() const;
  bool showWarningSymbol() const;

  void setRandomSeed(uint32_t seed);

private:
  Mood _baseMood;
  ExpressionId _expression;
  AttentionTarget _attention;
  
  uint32_t _expressionStartedMs;
  uint32_t _expressionEndsMs;
  uint32_t _lastInteractionMs;
  
  uint32_t _nextIdleActionMs;
  
  uint32_t _nextBlinkMs;
  uint32_t _blinkStartMs;
  uint8_t _blinkPhase; 
  bool _isDoubleBlink;

  bool _isMoving;

  float _eyeOpenLeft;
  float _eyeOpenRight;
  int8_t _pupilOffsetX;
  int8_t _pupilOffsetY;
  int8_t _browTiltLeft;
  int8_t _browTiltRight;
  int8_t _mouthOpenness;
  uint8_t _cheekIntensity;
  uint8_t _shakeAmount;
  uint8_t _bounceAmount;

  uint32_t _randomSeed;
  uint32_t randomRange(uint32_t minVal, uint32_t maxVal);

  void updateBlink(uint32_t nowMs, const PersonaManager& personaManager);
  void updateIdle(uint32_t nowMs, const PersonaManager& personaManager);
  void evaluateState(uint32_t nowMs);
};
