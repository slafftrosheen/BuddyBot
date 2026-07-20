#include "WifiControl.h"
#include "WebUiPage.h"
#include "arduino_secrets.h"

void WifiControl::begin(ControlRouter* router, RobotAPI* robot) {
  _router = router;
  _robot = robot;

  if (WIFI_START_SOFT_AP) {
    startSoftAP();
  }

  // NOTE: STA mode could be enabled here using arduino_secrets.h

  _server = new AsyncWebServer(WIFI_HTTP_PORT);
  _ws = new AsyncWebSocket(WIFI_WS_PATH);

  _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", WEB_UI_HTML);
  });

  _ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("WS Client connected: %u\n", client->id());
      String ack = _protocol.generateAck(0, true, "Connected", _robot->isArmed(), false);
      client->text(ack);
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("WS Client disconnected: %u\n", client->id());
      if (client->id() == _controllerId) {
        Serial.println("Controller disconnected. Stopping robot.");
        sendStop();
        _controllerId = 0;
      }
    } else if (type == WS_EVT_DATA) {
      handleWebSocketMessage(arg, data, len, client->id());
    }
  });

  _server->addHandler(_ws);
  _server->begin();
  Serial.println("WiFi Server started.");
}

void WifiControl::startSoftAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  Serial.print("SoftAP started: ");
  Serial.println(WIFI_AP_SSID);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void WifiControl::handleWebSocketMessage(void* arg, uint8_t* data, size_t len, uint32_t clientId) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String msg = (char*)data;

    RobotCommand cmd;
    bool isAuthCmd;
    String pin;
    uint32_t msgId;

    if (_protocol.parseCommand(msg.c_str(), cmd, isAuthCmd, pin, msgId)) {
      if (isAuthCmd) {
        if (pin == WIFI_UI_PIN) {
          _controllerId = clientId;
          String ack = _protocol.generateAck(msgId, true, "Control granted", _robot->isArmed(), 1);
          _ws->text(clientId, ack);
        } else {
          String ack = _protocol.generateAck(msgId, false, "Invalid PIN", _robot->isArmed(), 0);
          _ws->text(clientId, ack);
        }
      } else {
        if (clientId != _controllerId) {
          String ack = _protocol.generateAck(msgId, false, "Not controller", _robot->isArmed());
          _ws->text(clientId, ack);
          return;
        }

        bool ok = false;
        String replyMsg = "";
        
        if (cmd.kind == CommandKind::ARM && !ALLOW_MOTOR_ARMING) {
           replyMsg = "Rejected: motors are disabled in firmware";
        } else {
           ok = _router->execute(cmd);
           if (cmd.kind == CommandKind::MOVE) {
             _lastDriveCommandMs = millis();
           }
        }

        String ack = _protocol.generateAck(msgId, ok, replyMsg, _robot->isArmed(), 1);
        _ws->text(clientId, ack);
      }
    } else {
      String ack = _protocol.generateAck(0, false, "Parse error", _robot->isArmed());
      _ws->text(clientId, ack);
    }
  }
}

void WifiControl::sendStop() {
  RobotCommand stopCmd;
  stopCmd.kind = CommandKind::STOP;
  stopCmd.source = ControlSource::WIFI;
  _router->execute(stopCmd);
}

void WifiControl::update() {
  if (!_server) return;

  _ws->cleanupClients();

  uint32_t now = millis();
  
  if (_controllerId != 0 && _lastDriveCommandMs != 0) {
    if (now - _lastDriveCommandMs > WIFI_DRIVE_KEEPALIVE_MS * 2) { 
      sendStop();
      _lastDriveCommandMs = 0; 
    }
  }

  if (now - _lastTelemetryMs > 250) {
    _lastTelemetryMs = now;
    if (_ws->count() > 0) {
      broadcastTelemetry();
    }
  }
}

void WifiControl::broadcastTelemetry() {
  TelemetryData t;
  t.isConnected = true;
  t.isArmed = _robot->isArmed();
  t.mood = _robot->getMood();
  t.driveMode = DriveMode::STOPPED; 
  t.actionId = _robot->currentAction();
  RangeReading r = _robot->rangeReading();
  t.rangeMm = r.valid ? r.distanceMm : 0;
  t.hasController = (_controllerId != 0);

  String telemetryJson = _protocol.generateTelemetry(t);
  _ws->textAll(telemetryJson);
}
