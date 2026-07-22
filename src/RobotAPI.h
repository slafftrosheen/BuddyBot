#pragma once
#include "Types.h"
#include "Persona.h"
#include "RobotHal.h"
#include "RobotActions.h"
#include "ExpressionEngine.h"
#include "ActuatorState.h"
#include "ServoDiagnostics.h"
#include "ObstacleSafety.h"

class RobotAPI {
public:
  void begin(PersonaManager* persona, RobotHal* hal, RobotActions* actions);
  void update();

  Mood getMood() const;
  Mood baseMood() const;
  void setMood(Mood mood, bool playSound = true);
  void nextMood();
  void nextPersona();
  const char* personaName() const;

  void armMotors();
  void disarmMotors();
  void stopAll();

  bool isArmed() const;
  ActionId currentAction() const;

  bool driveAvailable() const;
  bool fourWheelDriveConfigured() const;
  bool manipulatorAvailable(ServoRole role) const;
  
  ActuatorCapabilities actuatorCapabilities() const;
  ManipulatorState manipulatorState(ManipulatorId id) const;

  void moveJointTo(ServoRole role, int16_t angle, uint16_t durationMs = 500);
  void restJoint(ServoRole role);
  void setAccessoryPosition(uint8_t index, bool active);

  bool move(DriveMode mode, uint16_t durationMs);
  void action(ActionId actionId);

  void playExpression(ExpressionId expression, uint16_t durationMs = 0);
  void setAttention(AttentionTarget target);
  ExpressionId expression() const;
  const ExpressionEngine& expressionEngine() const;
  RangeReading rangeReading() const;
  bool obstacleDetected() const;

  const RobotBuildConfig& buildConfig() const;

  void setAutonomyEnabled(bool enabled);
  bool autonomyEnabled() const;

  void rememberDriveCommand(const DriveCommand& cmd);
  bool lastDriveCommand(DriveCommand& out) const;
  void clearRememberedDriveCommand();

  const ObstacleSafetyStatus& obstacleSafetyStatus() const;
  bool forwardMotionAllowed() const;
  SafetyStopReason lastSafetyStopReason() const;
  
  RangeSensorHealth rangeSensorHealth() const;
  uint16_t rangeConsecutiveInvalid() const;

  ServoDiagnostics* diagnostics();

private:
  PersonaManager* _persona = nullptr;
  RobotHal* _hal = nullptr;
  RobotActions* _actions = nullptr;
  ExpressionEngine _expressions;
  ServoDiagnostics _diagnostics;
  ObstacleSafety _safety;
  Mood _mood = Mood::IDLE;
  bool _autonomyEnabled = false;

  DriveCommand _lastManualDriveCmd = {DriveMode::STOPPED, 0, 0, 0};
  uint32_t _lastManualDriveCmdMs = 0;
  bool _hasSavedCmd = false;

  uint32_t _lastSafetyStopMs = 0;
  const char* _lastSafetyStopReason = "";

  void playMoodSound();
  void playTone(uint16_t frequency, uint16_t durationMs);
};
