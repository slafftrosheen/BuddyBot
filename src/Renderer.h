#pragma once

#include <M5Unified.h>

#include "Config.h"
#include "Types.h"
#include "Persona.h"

class RobotRenderer {
public:
  void begin();

  void update(
    const PersonaManager& persona,
    Mood mood,
    bool motorsArmed,
    DriveMode driveMode
  );

private:
  M5Canvas _canvas = M5Canvas(&M5.Display);

  uint32_t _lastFrameMs = 0;
  uint32_t _nextBlinkMs = 0;
  uint32_t _blinkStartedMs = 0;
  uint32_t _nextLookMs = 0;

  bool _blinking = false;

  int _pupilX = 0;
  int _pupilY = 0;
  int _targetPupilX = 0;
  int _targetPupilY = 0;

  void draw(
    const PersonaManager& persona,
    Mood mood,
    bool motorsArmed,
    DriveMode driveMode
  );

  void drawFrame(uint16_t accent);
  void drawHeader(const PersonaProfile& profile, uint16_t accent);
  void drawAccessory(const PersonaProfile& profile);

  void drawEye(
    int centerX,
    int centerY,
    float openAmount,
    uint16_t accent
  );

  void drawBrows(Mood mood, uint16_t accent);
  void drawCheeks(Mood mood, int bounce, uint16_t accent);
  void drawMouth(Mood mood, uint16_t accent);

  void drawStatus(
    const PersonaManager& persona,
    Mood mood,
    bool motorsArmed,
    DriveMode driveMode,
    uint16_t accent
  );

  void drawDriveIndicator(DriveMode driveMode, uint16_t accent);

  int clampInt(int value, int minimum, int maximum);
};
