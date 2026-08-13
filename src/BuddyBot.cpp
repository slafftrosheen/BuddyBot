#include <M5Unified.h>

#include "Config.h"
#include "Types.h"
#include "Persona.h"
#include "Renderer.h"

#include "BuildProfiles.h"
#include "RobotHal.h"
#include "RobotActions.h"
#include "RobotAPI.h"

#include "ControlTypes.h"
#include "ControlProtocol.h"
#include "CommandParser.h"
#include "ControlRouter.h"
#include "SafetySupervisor.h"
#include "AutonomyManager.h"
#include "SystemStatus.h"
#include "WifiControl.h"
#include "BootDiagnostics.h"
#include "SystemHealth.h"

PersonaManager persona;
RobotRenderer renderer;
RobotHal hal;
RobotActions actions;
RobotAPI robot;

CommandParser parser;
ControlProtocol protocol;
ControlRouter router;
SafetySupervisor safety;
AutonomyManager autonomy;
SystemStatus systemStatus;
WifiControl wifiControl;
BootDiagnostics bootDiag;
SystemHealth systemHealth;

String serialBuffer;

void setup() {
  auto config = M5.config();
  config.fallback_board = m5::board_t::board_M5StickS3;
  M5.begin(config);

  M5.Display.wakeup();
  M5.Display.setBrightness(255);
  M5.Display.setRotation(DISPLAY_ROTATION);
  M5.Speaker.setVolume(75);

  // Screen test sequence at boot
  M5.Display.fillScreen(TFT_RED);
  delay(150);
  M5.Display.fillScreen(TFT_GREEN);
  delay(150);
  M5.Display.fillScreen(TFT_BLUE);
  delay(150);
  M5.Display.fillScreen(TFT_BLACK);

  Serial.begin(115200);
  randomSeed(esp_random());
  systemHealth.begin();

  SafetyPolicyConfig safetyConfig;
  safetyConfig.manualOverrideMs = SAFETY_MANUAL_OVERRIDE_MS;
  safetyConfig.imuMaxSampleAgeMs = IMU_MAX_SAMPLE_AGE_MS;
  safetyConfig.imuMinimumAccelG = IMU_MIN_ACCEL_G;
  safetyConfig.imuMaximumAccelG = IMU_MAX_ACCEL_G;
  safetyConfig.imuMaximumTiltDeg = IMU_MAX_TILT_DEG;
  safetyConfig.imuMaximumGyroDps = IMU_MAX_GYRO_DPS;
  safety.configure(safetyConfig);
  safety.begin();

  persona.begin();
  renderer.begin();

  RobotBuildConfig activeBuild = getActiveBuildConfig();
  if (!hal.begin(activeBuild)) {
    Serial.println("HAL initialization failed; motion hardware is unavailable.");
  }

  robot.begin(&persona, &hal, &actions);
  robot.setSafetySupervisor(&safety);
  robot.setBootDiagnostics(&bootDiag);

  autonomy.begin(&robot);
  systemStatus.begin(&robot, &hal, &bootDiag, &systemHealth);
  router.begin(&robot, &systemStatus, &safety);
  router.setDriveBase(hal.drive());
  router.setAutonomyManager(&autonomy);
  router.setWifiControl(&wifiControl);

  wifiControl.begin(&router, &robot, &systemStatus);
  if (ENABLE_WIFI_CONTROL) {
    if (!wifiControl.start()) {
      Serial.println("WIFI unavailable");
    }
  }

  robot.setMood(Mood::IDLE, false);

  Serial.println();
  Serial.println(FIRMWARE_NAME);
  Serial.println(FIRMWARE_VERSION);
  Serial.printf("Build profile: %s\n", getActiveBuildName());
  Serial.printf("External Grove I2C: SDA=%d SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
  
  Serial.println("--- SAFE DEFAULTS ---");
  Serial.printf("ALLOW_MOTOR_ARMING: %s\n", ALLOW_MOTOR_ARMING ? "true" : "false");
  Serial.printf("ENABLE_AUTONOMY_AT_BOOT: %s\n", ENABLE_AUTONOMY_AT_BOOT ? "true" : "false");
  Serial.printf("ENABLE_CAUTIOUS_ROAM: %s\n", ENABLE_CAUTIOUS_ROAM ? "true" : "false");
  Serial.println("---------------------");
  
  protocol.printHelp();
  
  bootDiag.begin(&robot);
}

void handleButtons() {
  bool aClicked = M5.BtnA.wasClicked();
  bool bClicked = M5.BtnB.wasClicked();
  bool aPressed = M5.BtnA.isPressed();
  bool bPressed = M5.BtnB.isPressed();
  
  static uint32_t chordStart = 0;
  static bool pairingRequested = false;
  static bool physicalActionTriggered = false;

  if (aPressed && bPressed) {
    if (chordStart == 0) {
      chordStart = millis();
      pairingRequested = false;
      physicalActionTriggered = false;
    }

    const uint32_t heldMs = millis() - chordStart;
    if (router.safetyState() == SafetyState::ESTOP) {
      if (!physicalActionTriggered && heldMs >= PHYSICAL_ESTOP_RESET_HOLD_MS) {
        router.clearPhysicalEmergencyStop();
        physicalActionTriggered = true;
      }
    } else {
      if (!physicalActionTriggered && heldMs >= PHYSICAL_ESTOP_HOLD_MS) {
        router.physicalEmergencyStop();
        physicalActionTriggered = true;
      } else if (!pairingRequested && heldMs >= 350 &&
                 wifiControl.running() && !wifiControl.controllerPresent()) {
        wifiControl.requestNewPairingCode();
        pairingRequested = true;
      }
    }

    return;
  }

  const bool suppressClicks = pairingRequested || physicalActionTriggered;
  if (!aPressed && !bPressed) {
    chordStart = 0;
    pairingRequested = false;
    physicalActionTriggered = false;
  }

  if (aClicked && !suppressClicks) {
    robot.nextMood();
  }
  if (bClicked && !suppressClicks) {
    robot.nextPersona();
  }
}

void handleSerial() {
  while (Serial.available()) {
    const char c = char(Serial.read());

    if (c == '\n' || c == '\r') {
      if (serialBuffer.length() > 0) {
        if (serialBuffer.equalsIgnoreCase("HELP")) {
          protocol.printHelp();
        } else {
          RobotCommand cmd;
          if (parser.parse(serialBuffer, cmd)) {
            cmd.source = ControlSource::SERIAL_CTRL;

            if (!router.execute(cmd)) {
              Serial.println("ERR Command denied by safety policy");
            }
          } else {
            Serial.println("ERR Unknown command. Type HELP");
          }
        }
        serialBuffer = "";
      }
      continue;
    }

    if (serialBuffer.length() < SERIAL_COMMAND_MAX_LENGTH) {
      serialBuffer += c;
    } else {
      serialBuffer = "";
      Serial.println("ERR Command too long");
    }
  }
}

void handleAutonomy() {
  RobotCommand cmd;
  if (autonomy.update(cmd)) {
    router.execute(cmd);
  }
}

void loop() {
  M5.update();
  handleButtons();
  
  if (!bootDiag.isComplete()) {
    bootDiag.update();
    if (!bootDiag.isComplete()) {
      systemHealth.feed();
      return;
    }
    router.completeBoot();
  }
  
  robot.update();
  router.updateSafety();
  handleAutonomy();

  if (ENABLE_WIFI_CONTROL) {
    wifiControl.update();
  }

  handleSerial();

  DriveMode mode = DriveMode::STOPPED;
  bool armed = false;

  if (hal.drive()) {
    mode = hal.drive()->driveMode();
    armed = hal.drive()->isArmed();
  }

  RenderState rState;
  rState.persona = &persona.current();
  rState.mood = robot.baseMood();
  rState.expression = robot.expression();
  rState.action = robot.currentAction();
  rState.motorsArmed = armed;
  rState.motorAllowedByFirmware = ALLOW_MOTOR_ARMING;
  rState.driveMode = mode;
  rState.wifiEnabled = systemStatus.wifiRunning();
  rState.wifiControllerConnected = systemStatus.wifiHasController();
  
  const char* pairingCode = systemStatus.getPairingCode();
  rState.pairingAvailable = systemStatus.wifiPairingAvailable() && (pairingCode && pairingCode[0] != '\0');
  rState.pairingCode = pairingCode;
  
  rState.apSsid = systemStatus.wifiSsid();
  rState.apIp = systemStatus.wifiIp();
  
  RangeReading rr = robot.rangeReading();
  rState.rangeValid = rr.valid;
  rState.rangeMm = rr.distanceMm;
  rState.obstacleDetected = robot.obstacleDetected();
  rState.autonomyEnabled = robot.autonomyEnabled();
  rState.batteryPercent = M5.Power.getBatteryLevel();
  rState.batteryValid = true;
  rState.safetyState = robot.obstacleSafetyStatus().state;

  renderer.update(rState, robot.expressionEngine());
  systemHealth.feed();
}
