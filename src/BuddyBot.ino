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

  actions.begin(&hal);
  robot.begin(&persona, &hal, &actions);

  autonomy.begin(&robot);
  systemStatus.begin(&robot, &hal);
  router.begin(&robot, &systemStatus);

  if (ENABLE_WIFI_CONTROL) {
    wifiControl.begin(&router, &robot, &systemStatus);
  }

  robot.setMood(Mood::IDLE, false);

  Serial.println();
  Serial.println(FIRMWARE_NAME);
  Serial.println(FIRMWARE_VERSION);
  Serial.printf("Build profile: %s\n", getActiveBuildName());
  Serial.printf("External Grove I2C: SDA=%d SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
  protocol.printHelp();
}

void handleButtons() {
  if (M5.BtnA.wasClicked()) {
    robot.nextMood();
  }

  if (M5.BtnB.wasClicked()) {
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

  handleButtons();
  handleSerial();

  robot.update();
  handleAutonomy();

  if (ENABLE_WIFI_CONTROL) {
    wifiControl.update();
    renderer.setWifiStatus(ENABLE_WIFI_CONTROL, systemStatus.getWifiSsid(), systemStatus.getWifiIp(), systemStatus.getWifiHasController());
    renderer.setPairingCode(systemStatus.getPairingCode());
  }

  DriveMode mode = DriveMode::STOPPED;
  bool armed = false;

  if (hal.drive()) {
    mode = hal.drive()->driveMode();
    armed = hal.drive()->isArmed();
  }

  renderer.update(
    persona,
    robot.getMood(),
    armed,
    mode
  );

  delay(2);
}
