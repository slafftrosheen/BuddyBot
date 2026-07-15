#include "Renderer.h"
#include <math.h>

// ------------------------------------------------------------
// Renderer-only colour palette.
// All names begin with UI_ to avoid M5GFX namespace conflicts.
// ------------------------------------------------------------

static constexpr uint16_t UI_BLACK      = 0x0000;
static constexpr uint16_t UI_WHITE      = 0xFFFF;
static constexpr uint16_t UI_BG_DARK    = 0x0092;
static constexpr uint16_t UI_PANEL      = 0x1146;
static constexpr uint16_t UI_PANEL_DARK = 0x08C4;
static constexpr uint16_t UI_ALERT_RED  = 0xF986;
static constexpr uint16_t UI_DIM_GREY   = 0x8410;

// ------------------------------------------------------------
// Setup
// ------------------------------------------------------------

void RobotRenderer::begin() {
  _canvas.setColorDepth(16);
  _canvas.createSprite(SCREEN_W, SCREEN_H);

  const uint32_t now = millis();

  _lastFrameMs = now;
  _nextBlinkMs = now + 1200;
  _nextLookMs = now + 1500;

  _blinking = false;

  _pupilX = 0;
  _pupilY = 0;
  _targetPupilX = 0;
  _targetPupilY = 0;
}

// ------------------------------------------------------------
// Animation update
// ------------------------------------------------------------

void RobotRenderer::update(
  const PersonaManager& persona,
  Mood mood,
  bool motorsArmed,
  DriveMode driveMode
) {
  const uint32_t now = millis();

  if (now - _lastFrameMs < FRAME_INTERVAL_MS) {
    return;
  }

  _lastFrameMs = now;

  // Natural blink sequence.
  if (!_blinking && now >= _nextBlinkMs) {
    _blinking = true;
    _blinkStartedMs = now;
  }

  if (_blinking && (now - _blinkStartedMs > 160)) {
    _blinking = false;
    _nextBlinkMs = now + 2200 + random(0, 1800);
  }

  // Idle eyes occasionally look around.
  if (mood == Mood::IDLE && now >= _nextLookMs) {
    _targetPupilX = random(-6, 7);
    _targetPupilY = random(-3, 4);
    _nextLookMs = now + 1200 + random(0, 1800);
  }

  // Mood-directed eye movement.
  if (mood == Mood::CURIOUS) {
    _targetPupilX = 5;
    _targetPupilY = -2;
  } else if (mood == Mood::SLEEPY) {
    _targetPupilX = 0;
    _targetPupilY = 4;
  } else if (mood == Mood::ALERT) {
    _targetPupilX = 0;
    _targetPupilY = 0;
  }

  // Smooth movement, one pixel per render frame.
  if (_pupilX < _targetPupilX) _pupilX++;
  if (_pupilX > _targetPupilX) _pupilX--;

  if (_pupilY < _targetPupilY) _pupilY++;
  if (_pupilY > _targetPupilY) _pupilY--;

  draw(persona, mood, motorsArmed, driveMode);
}

// ------------------------------------------------------------
// Main complete-frame render
// ------------------------------------------------------------

void RobotRenderer::draw(
  const PersonaManager& persona,
  Mood mood,
  bool motorsArmed,
  DriveMode driveMode
) {
  const PersonaProfile& profile = persona.current();
  const uint16_t accent = persona.moodColor(mood);

  float eyeOpen = 1.0f;

  // Blink closes and opens both eyes.
  if (_blinking) {
    const uint32_t elapsed = millis() - _blinkStartedMs;

    if (elapsed < 70) {
      eyeOpen = 1.0f - (float(elapsed) / 70.0f);
    } else {
      eyeOpen = float(elapsed - 70) / 90.0f;
    }

    eyeOpen = constrain(eyeOpen, 0.0f, 1.0f);
  }

  // Sleepy character keeps eyes partly closed.
  if (mood == Mood::SLEEPY) {
    eyeOpen *= 0.38f;
  }

  // Excited character has a small body/eye bounce.
  int bounce = 0;

  if (mood == Mood::EXCITED) {
    bounce = int(sinf(millis() * 0.018f) * 3.0f);
  }

  drawFrame(accent);
  drawHeader(profile, accent);
  drawAccessory(profile);

  drawBrows(mood, accent);

  drawEye(39, 106 + bounce, eyeOpen, accent);
  drawEye(96, 106 + bounce, eyeOpen, accent);

  drawCheeks(mood, bounce, accent);
  drawMouth(mood, accent);

  drawStatus(persona, mood, motorsArmed, driveMode, accent);

  // Send one finished frame to the LCD: reduced visible flicker.
  _canvas.pushSprite(0, 0);
}

// ------------------------------------------------------------
// Chassis, frame and header
// ------------------------------------------------------------

void RobotRenderer::drawFrame(uint16_t accent) {
  _canvas.fillScreen(UI_BG_DARK);

  // Outer robot device housing.
  _canvas.drawRoundRect(2, 2, 131, 236, 9, accent);
  _canvas.drawRoundRect(4, 4, 127, 232, 8, UI_PANEL_DARK);

  // Inner screen plate.
  _canvas.fillRoundRect(7, 7, 121, 226, 7, UI_PANEL);

  // Bottom separator.
  _canvas.drawFastHLine(12, 216, 111, accent);

  // Four stylised mounting bolts.
  _canvas.fillCircle(13, 13, 2, accent);
  _canvas.fillCircle(122, 13, 2, accent);
  _canvas.fillCircle(13, 227, 2, accent);
  _canvas.fillCircle(122, 227, 2, accent);
}

void RobotRenderer::drawHeader(
  const PersonaProfile& profile,
  uint16_t accent
) {
  // Header plate.
  _canvas.fillRoundRect(8, 8, 119, 25, 5, UI_BLACK);
  _canvas.drawRoundRect(8, 8, 119, 25, 5, accent);

  _canvas.setTextDatum(middle_left);
  _canvas.setTextSize(1);
  _canvas.setTextColor(accent, UI_BLACK);
  _canvas.drawString(profile.name, 14, 20);

  _canvas.setTextDatum(middle_right);
  _canvas.setTextSize(1);
  _canvas.setTextColor(UI_WHITE, UI_BLACK);
  _canvas.drawString(profile.subtitle, 118, 20);

  // Decorative battery/activity module.
  _canvas.drawRoundRect(103, 37, 18, 8, 2, accent);
  _canvas.fillRect(121, 40, 2, 3, accent);
  _canvas.fillRect(106, 40, 10, 3, accent);
}

// ------------------------------------------------------------
// Persona accessories
// ------------------------------------------------------------

void RobotRenderer::drawAccessory(const PersonaProfile& profile) {
  const uint16_t accent = profile.colors.accent;

  // Head accessory.
  switch (profile.headAccessory) {
    case Accessory::ANTENNA:
      _canvas.drawLine(67, 49, 67, 36, accent);
      _canvas.drawLine(68, 49, 68, 36, accent);
      _canvas.fillCircle(67, 32, 5, accent);
      _canvas.fillCircle(67, 32, 2, UI_WHITE);
      break;

    case Accessory::VISOR:
      _canvas.fillRoundRect(26, 47, 83, 10, 4, accent);
      _canvas.fillRoundRect(30, 49, 75, 6, 3, UI_BLACK);
      _canvas.drawFastHLine(37, 52, 61, UI_WHITE);
      break;

    case Accessory::HAT:
      _canvas.fillRoundRect(38, 39, 58, 14, 4, accent);
      _canvas.fillRoundRect(27, 50, 79, 7, 3, accent);
      _canvas.fillRoundRect(43, 42, 48, 6, 2, UI_BLACK);
      break;

    case Accessory::EARS:
      _canvas.fillRoundRect(10, 81, 14, 32, 5, accent);
      _canvas.fillRoundRect(111, 81, 14, 32, 5, accent);
      _canvas.fillRoundRect(14, 86, 6, 22, 2, UI_BLACK);
      _canvas.fillRoundRect(115, 86, 6, 22, 2, UI_BLACK);
      break;

    default:
      break;
  }

  // Side accessory.
  switch (profile.sideAccessory) {
    case Accessory::STAR_BADGE:
      _canvas.fillCircle(114, 58, 7, accent);
      _canvas.fillCircle(114, 58, 3, UI_WHITE);
      _canvas.drawFastHLine(106, 58, 16, UI_WHITE);
      _canvas.drawFastVLine(114, 50, 16, UI_WHITE);
      break;

    case Accessory::BOW:
      _canvas.fillTriangle(103, 56, 116, 48, 116, 64, accent);
      _canvas.fillTriangle(129, 56, 116, 48, 116, 64, accent);
      _canvas.fillCircle(116, 56, 4, UI_WHITE);
      break;

    case Accessory::HAT:
      _canvas.fillRoundRect(98, 49, 27, 9, 3, accent);
      _canvas.fillCircle(104, 46, 4, accent);
      break;

    default:
      break;
  }
}

// ------------------------------------------------------------
// Face graphics
// ------------------------------------------------------------

void RobotRenderer::drawEye(
  int centerX,
  int centerY,
  float openAmount,
  uint16_t accent
) {
  constexpr int EYE_WIDTH = 45;

  int eyeHeight = int(36 * openAmount);

  if (eyeHeight < 5) {
    // Closed eyelid.
    _canvas.drawLine(centerX - 18, centerY,
                     centerX + 18, centerY, accent);

    _canvas.drawLine(centerX - 18, centerY + 1,
                     centerX + 18, centerY + 1, accent);
    return;
  }

  // Outer glowing eye casing.
  _canvas.fillRoundRect(
    centerX - EYE_WIDTH / 2,
    centerY - eyeHeight / 2,
    EYE_WIDTH,
    eyeHeight,
    8,
    accent
  );

  // Black mechanical bezel.
  _canvas.fillRoundRect(
    centerX - EYE_WIDTH / 2 + 3,
    centerY - eyeHeight / 2 + 3,
    EYE_WIDTH - 6,
    eyeHeight - 6,
    6,
    UI_BLACK
  );

  // White eye panel.
  _canvas.fillRoundRect(
    centerX - EYE_WIDTH / 2 + 6,
    centerY - eyeHeight / 2 + 6,
    EYE_WIDTH - 12,
    eyeHeight - 12,
    4,
    UI_WHITE
  );

  // Pupil and highlight.
  const int pupilCenterX = clampInt(
    centerX + _pupilX,
    centerX - 10,
    centerX + 10
  );

  const int pupilCenterY = clampInt(
    centerY + _pupilY,
    centerY - 7,
    centerY + 7
  );

  _canvas.fillCircle(pupilCenterX, pupilCenterY, 9, accent);
  _canvas.fillCircle(pupilCenterX, pupilCenterY, 5, UI_BLACK);
  _canvas.fillCircle(pupilCenterX - 3, pupilCenterY - 3, 2, UI_WHITE);

  // Small lower eye scan bar.
  _canvas.drawFastHLine(
    centerX - 11,
    centerY + eyeHeight / 2 - 5,
    23,
    accent
  );
}

void RobotRenderer::drawBrows(Mood mood, uint16_t accent) {
  switch (mood) {
    case Mood::HAPPY:
      _canvas.drawLine(20, 68, 51, 63, accent);
      _canvas.drawLine(20, 69, 51, 64, accent);
      _canvas.drawLine(84, 63, 115, 68, accent);
      _canvas.drawLine(84, 64, 115, 69, accent);
      break;

    case Mood::CURIOUS:
      _canvas.drawLine(20, 68, 51, 60, accent);
      _canvas.drawLine(84, 62, 115, 65, accent);
      break;

    case Mood::SLEEPY:
      _canvas.drawLine(20, 69, 51, 71, accent);
      _canvas.drawLine(84, 71, 115, 69, accent);
      break;

    case Mood::EXCITED:
      _canvas.drawLine(20, 67, 51, 59, accent);
      _canvas.drawLine(84, 59, 115, 67, accent);
      break;

    case Mood::ALERT:
      _canvas.drawLine(20, 59, 51, 70, accent);
      _canvas.drawLine(20, 60, 51, 71, accent);
      _canvas.drawLine(84, 70, 115, 59, accent);
      _canvas.drawLine(84, 71, 115, 60, accent);
      break;

    default:
      break;
  }
}

void RobotRenderer::drawCheeks(
  Mood mood,
  int bounce,
  uint16_t accent
) {
  if (mood == Mood::HAPPY || mood == Mood::EXCITED) {
    _canvas.fillCircle(22, 143 + bounce, 4, accent);
    _canvas.fillCircle(113, 143 + bounce, 4, accent);

    _canvas.fillCircle(22, 143 + bounce, 1, UI_WHITE);
    _canvas.fillCircle(113, 143 + bounce, 1, UI_WHITE);
  }

  if (mood == Mood::ALERT && ((millis() / 180) % 2 == 0)) {
    _canvas.fillCircle(22, 143, 5, UI_ALERT_RED);
    _canvas.fillCircle(113, 143, 5, UI_ALERT_RED);
    _canvas.fillCircle(22, 143, 2, UI_WHITE);
    _canvas.fillCircle(113, 143, 2, UI_WHITE);
  }
}

void RobotRenderer::drawMouth(Mood mood, uint16_t accent) {
  constexpr int MOUTH_Y = 158;

  // Mouth display housing.
  _canvas.fillRoundRect(34, 140, 67, 37, 7, UI_BLACK);
  _canvas.drawRoundRect(35, 141, 65, 35, 6, accent);

  switch (mood) {
    case Mood::HAPPY:
      _canvas.drawLine(48, MOUTH_Y, 53, MOUTH_Y + 5, accent);
      _canvas.drawLine(53, MOUTH_Y + 5, 82, MOUTH_Y + 5, accent);
      _canvas.drawLine(82, MOUTH_Y + 5, 87, MOUTH_Y, accent);
      _canvas.drawLine(48, MOUTH_Y + 1, 53, MOUTH_Y + 6, accent);
      _canvas.drawLine(82, MOUTH_Y + 6, 87, MOUTH_Y + 1, accent);
      break;

    case Mood::CURIOUS:
      _canvas.drawCircle(67, MOUTH_Y + 3, 7, accent);
      _canvas.drawCircle(67, MOUTH_Y + 3, 4, UI_BLACK);
      break;

    case Mood::SLEEPY:
      _canvas.drawFastHLine(52, MOUTH_Y + 5, 31, accent);
      _canvas.drawFastHLine(55, MOUTH_Y + 6, 25, accent);
      break;

    case Mood::EXCITED: {
      const int pulse = int((sinf(millis() * 0.012f) + 1.0f) * 3.0f);

      _canvas.fillRoundRect(
        52,
        MOUTH_Y - pulse,
        31,
        14 + pulse,
        3,
        accent
      );

      _canvas.fillRoundRect(
        57,
        MOUTH_Y - pulse + 3,
        21,
        7 + pulse,
        2,
        UI_BLACK
      );
      break;
    }

    case Mood::ALERT:
      _canvas.fillRect(50, MOUTH_Y, 35, 6, accent);
      _canvas.fillRect(60, MOUTH_Y - 6, 5, 18, accent);
      _canvas.fillRect(70, MOUTH_Y - 6, 5, 18, accent);
      break;

    default:
      _canvas.drawFastHLine(52, MOUTH_Y + 3, 31, accent);
      break;
  }
}

// ------------------------------------------------------------
// Status area
// ------------------------------------------------------------

void RobotRenderer::drawStatus(
  const PersonaManager& persona,
  Mood mood,
  bool motorsArmed,
  DriveMode driveMode,
  uint16_t accent
) {
  _canvas.fillRoundRect(11, 187, 113, 27, 6, UI_BLACK);
  _canvas.drawRoundRect(12, 188, 111, 25, 5, accent);

  _canvas.setTextDatum(middle_center);
  _canvas.setTextSize(1);

  _canvas.setTextColor(accent, UI_BLACK);
  _canvas.drawString(persona.messageFor(mood), 67, 199);

  drawDriveIndicator(driveMode, accent);

  _canvas.setTextColor(
    motorsArmed ? accent : UI_DIM_GREY,
    UI_PANEL
  );

  if (motorsArmed) {
    _canvas.drawString("MOTORS ARMED", 67, 223);
  } else {
    _canvas.drawString("SAFE SCREEN MODE", 67, 223);
  }
}

void RobotRenderer::drawDriveIndicator(
  DriveMode driveMode,
  uint16_t accent
) {
  const int centerX = 113;
  const int centerY = 198;

  if (driveMode == DriveMode::STOPPED) {
    _canvas.drawCircle(centerX, centerY, 5, UI_DIM_GREY);
    _canvas.fillCircle(centerX, centerY, 2, UI_DIM_GREY);
    return;
  }

  if (driveMode == DriveMode::FORWARD) {
    _canvas.fillTriangle(
      centerX, centerY - 6,
      centerX - 5, centerY + 4,
      centerX + 5, centerY + 4,
      accent
    );
    return;
  }

  if (driveMode == DriveMode::REVERSE) {
    _canvas.fillTriangle(
      centerX, centerY + 6,
      centerX - 5, centerY - 4,
      centerX + 5, centerY - 4,
      accent
    );
    return;
  }

  if (driveMode == DriveMode::TURN_LEFT) {
    _canvas.fillTriangle(
      centerX - 6, centerY,
      centerX + 4, centerY - 5,
      centerX + 4, centerY + 5,
      accent
    );
    return;
  }

  if (driveMode == DriveMode::TURN_RIGHT) {
    _canvas.fillTriangle(
      centerX + 6, centerY,
      centerX - 4, centerY - 5,
      centerX - 4, centerY + 5,
      accent
    );
  }
}

// ------------------------------------------------------------
// Utility
// ------------------------------------------------------------

int RobotRenderer::clampInt(
  int value,
  int minimum,
  int maximum
) {
  if (value < minimum) return minimum;
  if (value > maximum) return maximum;
  return value;
}
