#include "Renderer.h"
#include <math.h>

void RobotRenderer::begin() {
  _canvas.createSprite(SCREEN_W, SCREEN_H);
  _particles.begin();
}

int RobotRenderer::clampInt(int value, int minimum, int maximum) {
  if (value < minimum) return minimum;
  if (value > maximum) return maximum;
  return value;
}

void RobotRenderer::update(const RenderState& state, const ExpressionEngine& expressions) {
  uint32_t nowMs = millis();
  if (nowMs - _lastFrameMs < FRAME_INTERVAL_MS) {
    return;
  }
  _lastFrameMs = nowMs;

  // Hysteresis
  if (state.batteryValid != _lastBatteryValid || abs(state.batteryPercent - _lastBatteryPercent) >= BATTERY_UI_UPDATE_DELTA_PERCENT) {
    _lastBatteryValid = state.batteryValid;
    _lastBatteryPercent = state.batteryPercent;
  }
  
  if (state.rangeValid != _lastRangeValid || abs((int)state.rangeMm - (int)_lastRangeMm) >= RANGE_UI_UPDATE_DELTA_MM) {
    _lastRangeValid = state.rangeValid;
    _lastRangeMm = state.rangeMm;
  }

  // Draw
  draw(state, expressions);
}

void RobotRenderer::draw(const RenderState& state, const ExpressionEngine& expressions) {
  _canvas.fillSprite(state.persona->panelColor); // default background color (border)
  
  // Inner face background
  _canvas.fillRect(2, 2, SCREEN_W - 4, SCREEN_H - 4, state.persona->colors.background);

  // If pairing is available, do not draw particles to avoid overlapping
  if (!state.pairingAvailable) {
    _particles.setEnabled(ENABLE_PERSONA_PARTICLES && state.expression != ExpressionId::OBSTACLE && state.expression != ExpressionId::SCARED && state.expression != ExpressionId::LOW_BATTERY);
    _particles.update();
    _particles.draw(_canvas);
  } else {
    _particles.setEnabled(false);
  }

  drawSystemRail(state);
  drawPersonaBadge(state);
  drawFace(state, expressions);
  drawExpressionOverlays(state, expressions);
  drawMessagePanel(state);

  if (state.pairingAvailable) {
    drawPairingOverlay(state);
  }

  _canvas.pushSprite(0, 0);
}

void RobotRenderer::drawSystemRail(const RenderState& state) {
  _canvas.fillRect(0, 0, SCREEN_W, 20, state.persona->panelColor);

  // WiFi icon
  if (state.wifiEnabled) {
    uint16_t wifiColor = state.wifiControllerConnected ? TFT_GREEN : TFT_WHITE;
    _canvas.fillCircle(10, 10, 4, wifiColor);
    _canvas.drawCircle(10, 10, 6, wifiColor);
  }

  // Motor state
  uint16_t motorColor = state.motorsArmed ? TFT_RED : (state.motorAllowedByFirmware ? TFT_WHITE : TFT_DARKGREY);
  _canvas.fillRect(24, 6, 12, 8, motorColor);
  
  // Battery
  if (_lastBatteryValid) {
    _canvas.drawRect(SCREEN_W - 30, 6, 20, 8, TFT_WHITE);
    _canvas.fillRect(SCREEN_W - 10, 8, 2, 4, TFT_WHITE);
    uint16_t w = (_lastBatteryPercent * 18) / 100;
    uint16_t bColor = _lastBatteryPercent > 20 ? TFT_GREEN : TFT_RED;
    _canvas.fillRect(SCREEN_W - 29, 7, w, 6, bColor);
  }
}

void RobotRenderer::drawPersonaBadge(const RenderState& state) {
  _canvas.setTextDatum(top_center);
  _canvas.setTextColor(state.persona->textColor, state.persona->colors.background);
  _canvas.drawString(state.persona->name, SCREEN_W / 2, 24);
  
  // Draw accessory icon if any
  if (state.persona->headAccessory == Accessory::ANTENNA) {
    _canvas.drawLine(SCREEN_W/2 - 10, 24, SCREEN_W/2 - 10, 18, state.persona->accentGlowColor);
    if (state.persona->showAntennaPulse) {
      _canvas.fillCircle(SCREEN_W/2 - 10, 16, 2, state.persona->accentGlowColor);
    }
  }
}

void RobotRenderer::drawFace(const RenderState& state, const ExpressionEngine& expressions) {
  int cy = SCREEN_H / 2 - 10; // face center Y
  
  float leftEyeOpen = expressions.eyeOpenLeft();
  float rightEyeOpen = expressions.eyeOpenRight();
  
  // Action cues
  if (state.action == ActionId::SLEEP) {
    leftEyeOpen = 0.0f;
    rightEyeOpen = 0.0f;
  } else if (state.action == ActionId::DANCE || state.action == ActionId::CELEBRATE) {
    // Squint
    if (leftEyeOpen > 0.4f) leftEyeOpen = 0.4f;
    if (rightEyeOpen > 0.4f) rightEyeOpen = 0.4f;
  }

  // Draw Eyes
  drawEye(SCREEN_W / 2 - 30, cy, leftEyeOpen, expressions.pupilOffsetX(), expressions.pupilOffsetY(), state);
  drawEye(SCREEN_W / 2 + 30, cy, rightEyeOpen, expressions.pupilOffsetX(), expressions.pupilOffsetY(), state);

  // Draw Brows
  drawBrows(expressions.browTiltLeft(), expressions.browTiltRight(), state);

  // Draw Mouth
  drawMouth(expressions.mouthOpenness(), state);

  // Draw Cheeks
  drawCheeks(expressions.cheekIntensity(), expressions.bounceAmount(), state);
}

void RobotRenderer::drawEye(int centerX, int centerY, float openAmount, int8_t pupilOffsetX, int8_t pupilOffsetY, const RenderState& state) {
  int w = state.persona->eyeWidth;
  int h = (int)(state.persona->eyeHeight * openAmount);
  if (h < 2) h = 2; // minimum height
  
  int r = 4;
  if (state.persona->faceStyle == FaceStyle::ROUND_SOFT) {
    r = 8;
  } else if (state.persona->faceStyle == FaceStyle::VISOR_TECH) {
    r = 2;
  } else if (state.persona->faceStyle == FaceStyle::PIXEL_CUTE) {
    r = 0;
  }

  // Draw eye outline/fill
  _canvas.fillRoundRect(centerX - w/2, centerY - h/2, w, h, r, state.persona->colors.eye);

  // Draw pupil if eye is open enough
  if (h > 6) {
    int px = clampInt(centerX + pupilOffsetX, centerX - w/2 + state.persona->pupilRadius, centerX + w/2 - state.persona->pupilRadius);
    int py = clampInt(centerY + pupilOffsetY, centerY - h/2 + state.persona->pupilRadius, centerY + h/2 - state.persona->pupilRadius);
    _canvas.fillCircle(px, py, state.persona->pupilRadius, state.persona->colors.background);
    
    // Highlights
    if (state.persona->showEyeHighlights) {
       _canvas.fillCircle(px - 2, py - 2, 1, TFT_WHITE);
    }
  }

  // Scan lines
  if (state.persona->showScanLines && h > 4) {
    for (int y = centerY - h/2 + 2; y < centerY + h/2; y += 4) {
      _canvas.drawLine(centerX - w/2, y, centerX + w/2, y, state.persona->panelColor);
    }
  }
}

void RobotRenderer::drawBrows(int8_t tiltLeft, int8_t tiltRight, const RenderState& state) {
  if (tiltLeft == 0 && tiltRight == 0) return; // neutral, no brows
  int cy = SCREEN_H / 2 - 10 - state.persona->eyeHeight / 2 - 10;
  int lx = SCREEN_W / 2 - 30;
  int rx = SCREEN_W / 2 + 30;
  int bw = state.persona->eyeWidth + 4;
  
  _canvas.drawLine(lx - bw/2, cy + tiltLeft, lx + bw/2, cy - tiltLeft, state.persona->accentGlowColor);
  _canvas.drawLine(rx - bw/2, cy - tiltRight, rx + bw/2, cy + tiltRight, state.persona->accentGlowColor);
}

void RobotRenderer::drawMouth(int8_t openness, const RenderState& state) {
  int cy = SCREEN_H / 2 - 10 + state.persona->eyeHeight / 2 + 15;
  if (openness > 0) {
    _canvas.fillCircle(SCREEN_W / 2, cy, openness / 2, state.persona->accentGlowColor);
  } else if (openness < 0) {
    _canvas.drawLine(SCREEN_W / 2 - 10, cy + 5, SCREEN_W / 2 + 10, cy + 5, state.persona->accentGlowColor);
    _canvas.drawLine(SCREEN_W / 2 - 10, cy + 5, SCREEN_W / 2, cy, state.persona->accentGlowColor);
    _canvas.drawLine(SCREEN_W / 2 + 10, cy + 5, SCREEN_W / 2, cy, state.persona->accentGlowColor);
  } else {
    // Neutral mouth
    _canvas.drawLine(SCREEN_W / 2 - 10, cy, SCREEN_W / 2 + 10, cy, state.persona->accentGlowColor);
  }
}

void RobotRenderer::drawCheeks(uint8_t intensity, int bounce, const RenderState& state) {
  if (intensity > 0) {
    int cy = SCREEN_H / 2 - 10 + state.persona->eyeHeight / 2 + bounce;
    uint16_t cColor = _canvas.color565(intensity, 0, intensity/2); // approximate mix
    _canvas.fillCircle(SCREEN_W / 2 - 45, cy, 4, state.persona->cheekColor);
    _canvas.fillCircle(SCREEN_W / 2 + 45, cy, 4, state.persona->cheekColor);
  } else if (state.persona->showFreckles) {
    int cy = SCREEN_H / 2 - 10 + state.persona->eyeHeight / 2;
    _canvas.fillCircle(SCREEN_W / 2 - 45, cy, 1, state.persona->cheekColor);
    _canvas.fillCircle(SCREEN_W / 2 + 45, cy, 1, state.persona->cheekColor);
  }
}

void RobotRenderer::drawExpressionOverlays(const RenderState& state, const ExpressionEngine& expressions) {
  if (expressions.showTear()) {
    _canvas.fillCircle(SCREEN_W / 2 - 40, SCREEN_H / 2 + 10, 3, TFT_CYAN);
  }
  if (expressions.showSweatDrop()) {
    _canvas.fillCircle(SCREEN_W / 2 + 40, SCREEN_H / 2 - 20, 3, TFT_CYAN);
  }
  if (expressions.showQuestionMark()) {
    _canvas.setTextColor(state.persona->accentGlowColor);
    _canvas.drawString("?", SCREEN_W / 2 + 40, SCREEN_H / 2 - 40);
  }
  if (expressions.showExclamationMark()) {
    _canvas.setTextColor(state.persona->accentGlowColor);
    _canvas.drawString("!", SCREEN_W / 2 + 40, SCREEN_H / 2 - 40);
  }
  if (expressions.showSleepBubble()) {
    _canvas.setTextColor(TFT_WHITE);
    _canvas.drawString("zZ", SCREEN_W / 2 + 40, SCREEN_H / 2 - 40);
  }
  if (expressions.showThinkingDots()) {
    _canvas.fillCircle(SCREEN_W / 2 + 30, SCREEN_H / 2 - 40, 2, TFT_WHITE);
    _canvas.fillCircle(SCREEN_W / 2 + 40, SCREEN_H / 2 - 40, 2, TFT_WHITE);
    _canvas.fillCircle(SCREEN_W / 2 + 50, SCREEN_H / 2 - 40, 2, TFT_WHITE);
  }
  if (expressions.showListeningBars()) {
    _canvas.fillRect(SCREEN_W / 2 - 30, SCREEN_H / 2 + 30, 4, 10, state.persona->accentGlowColor);
    _canvas.fillRect(SCREEN_W / 2 - 20, SCREEN_H / 2 + 25, 4, 15, state.persona->accentGlowColor);
    _canvas.fillRect(SCREEN_W / 2 - 10, SCREEN_H / 2 + 20, 4, 20, state.persona->accentGlowColor);
  }
  if (expressions.showWarningSymbol()) {
    _canvas.fillTriangle(SCREEN_W / 2, SCREEN_H / 2 - 60, SCREEN_W / 2 - 20, SCREEN_H / 2 - 20, SCREEN_W / 2 + 20, SCREEN_H / 2 - 20, TFT_RED);
    _canvas.setTextColor(TFT_WHITE);
    _canvas.drawString("!", SCREEN_W / 2, SCREEN_H / 2 - 40);
  }
}

void RobotRenderer::drawMessagePanel(const RenderState& state) {
  _canvas.fillRect(0, SCREEN_H - 24, SCREEN_W, 24, state.persona->panelColor);
  _canvas.setTextDatum(middle_center);
  _canvas.setTextColor(state.persona->textColor);

  // If there's an obstacle, prioritize that message
  if (state.expression == ExpressionId::OBSTACLE || state.safetyState == ObstacleSafetyState::BLOCKED) {
    _canvas.setTextColor(TFT_RED);
    _canvas.drawString("BLOCKED", SCREEN_W / 2, SCREEN_H - 12);
  } else if (state.safetyState == ObstacleSafetyState::CAUTION) {
    _canvas.setTextColor(TFT_ORANGE);
    _canvas.drawString("CAUTION", SCREEN_W / 2, SCREEN_H - 12);
  } else if (state.expression == ExpressionId::LOW_BATTERY) {
    _canvas.setTextColor(TFT_RED);
    _canvas.drawString("LOW BATTERY", SCREEN_W / 2, SCREEN_H - 12);
  } else {
    // Show autonomy or range info
    if (state.autonomyEnabled) {
      _canvas.drawString("AUTO", SCREEN_W / 2, SCREEN_H - 12);
    } else {
      char statusBuf[32];
      if (_lastRangeValid) {
        snprintf(statusBuf, sizeof(statusBuf), "%d mm", _lastRangeMm);
      } else {
        snprintf(statusBuf, sizeof(statusBuf), "-- mm");
      }
      _canvas.drawString(statusBuf, SCREEN_W / 2, SCREEN_H - 12);
    }
  }
}

void RobotRenderer::drawPairingOverlay(const RenderState& state) {
  // Draw semi-transparent background (using dither or a dark rect)
  _canvas.fillRect(10, SCREEN_H / 2 - 30, SCREEN_W - 20, 60, TFT_BLACK);
  _canvas.drawRect(10, SCREEN_H / 2 - 30, SCREEN_W - 20, 60, state.persona->accentGlowColor);
  
  _canvas.setTextDatum(middle_center);
  _canvas.setTextColor(TFT_WHITE);
  _canvas.drawString("PAIRING CODE", SCREEN_W / 2, SCREEN_H / 2 - 15);
  
  _canvas.setTextColor(TFT_GREEN);
  // Increase font size conceptually by drawing scaled string
  _canvas.setTextSize(2);
  _canvas.drawString(state.pairingCode, SCREEN_W / 2, SCREEN_H / 2 + 10);
  _canvas.setTextSize(1);
}
