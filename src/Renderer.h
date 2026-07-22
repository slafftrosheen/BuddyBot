#pragma once

#include <M5Unified.h>

#include "Config.h"
#include "Types.h"
#include "Persona.h"
#include "ExpressionEngine.h"
#include "PersonaParticles.h"
#include "ObstacleSafety.h"

struct RenderState {
  const PersonaProfile* persona;
  Mood mood;
  ExpressionId expression;
  ActionId action;
  bool motorsArmed;
  bool motorAllowedByFirmware;
  DriveMode driveMode;
  bool wifiEnabled;
  bool wifiControllerConnected;
  bool pairingAvailable;
  const char* pairingCode;
  const char* apSsid;
  const char* apIp;
  bool rangeValid;
  uint16_t rangeMm;
  bool obstacleDetected;
  bool autonomyEnabled;
  uint8_t batteryPercent;
  bool batteryValid;
  ObstacleSafetyState safetyState;
};

class RobotRenderer {
public:
  void begin();

  void update(const RenderState& state, const ExpressionEngine& expressions);

private:
  M5Canvas _canvas = M5Canvas(&M5.Display);

  uint32_t _lastFrameMs = 0;
  
  // Hysteresis states
  uint8_t _lastBatteryPercent = 0;
  bool _lastBatteryValid = false;
  uint16_t _lastRangeMm = 0;
  bool _lastRangeValid = false;

  PersonaParticlePool _particles;

  void draw(const RenderState& state, const ExpressionEngine& expressions);
  
  void drawSystemRail(const RenderState& state);
  void drawPersonaBadge(const RenderState& state);
  void drawFace(const RenderState& state, const ExpressionEngine& expressions);
  void drawEye(int centerX, int centerY, float openAmount, int8_t pupilOffsetX, int8_t pupilOffsetY, const RenderState& state);
  void drawBrows(int8_t tiltLeft, int8_t tiltRight, const RenderState& state);
  void drawMouth(int8_t openness, const RenderState& state);
  void drawCheeks(uint8_t intensity, int bounce, const RenderState& state);
  
  void drawExpressionOverlays(const RenderState& state, const ExpressionEngine& expressions);
  void drawMessagePanel(const RenderState& state);
  void drawPairingOverlay(const RenderState& state);

  int clampInt(int value, int minimum, int maximum);
};
