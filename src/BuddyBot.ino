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
#include "AutonomyManager.h"
#include "SystemStatus.h"
#include "WifiControl.h"
#include "BootDiagnostics.h"

PersonaManager persona;
RobotRenderer renderer;
RobotHal hal;
RobotActions actions;
RobotAPI robot;

CommandParser parser;
ControlProtocol protocol;
ControlRouter router;
AutonomyManager autonomy;
SystemStatus systemStatus;
WifiControl wifiControl;
BootDiagnostics bootDiag;

String serialBuffer;

void setup() {
  auto config = M5.config();
  M5.begin(config);

  M5.Display.setRotation(DISPLAY_ROTATION);
  M5.Display.setBrightness(150);
  M5.Speaker.setVolume(75);

  Serial.begin(115200);
  randomSeed(esp_random());

  persona.begin();
  renderer.begin();

  RobotBuildConfig activeBuild = getActiveBuildConfig();
  hal.begin(activeBuild);

  robot.begin(&persona, &hal, &actions);

  autonomy.begin(&robot);
  systemStatus.begin(&robot, &hal, &bootDiag);
  router.begin(&robot, &systemStatus);
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
  static bool chordConsumed = false;

  if (aPressed && bPressed) {
    if (chordStart == 0) {
      chordStart = millis();
    } else if (!chordConsumed && (millis() - chordStart > 350)) {
      chordConsumed = true;
      if (wifiControl.running() && !wifiControl.controllerPresent()) {
        wifiControl.requestNewPairingCode();
      }
    }
  } else if (!aPressed && !bPressed) {
    chordStart = 0;
    chordConsumed = false;
  }

  if (aClicked && !chordConsumed) {
    robot.nextMood();
  }
  if (bClicked && !chordConsumed) {
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

            if (cmd.kind == CommandKind::AUTONOMY_SET) {
              autonomy.setEnabled(cmd.flag);
              robot.setAutonomyEnabled(cmd.flag);
            }

            router.execute(cmd);
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
  
  if (!bootDiag.isComplete()) {
    bootDiag.update();
    return;
  }
  
  robot.update();
  handleAutonomy();

  if (ENABLE_WIFI_CONTROL) {
    wifiControl.update();
  }

  handleButtons();
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
}
