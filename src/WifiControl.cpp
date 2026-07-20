#include "WifiControl.h"
#include "WebUiPage.h"
#include "arduino_secrets.h"
#include "BuildProfiles.h"

void WifiControl::begin(ControlRouter* router, RobotAPI* robot, SystemStatus* status) {
  _router = router;
  _robot = robot;
  _status = status;

  if (WIFI_START_SOFT_AP) {
    startSoftAP();
  }
  
  _status->setWifiStatus(WIFI_AP_SSID, WiFi.softAPIP().toString().c_str(), 0, false);
  generatePairingCode();
  logEvent("INFO", "wifi_started");

  _server = new AsyncWebServer(WIFI_HTTP_PORT);
  _ws = new AsyncWebSocket(WIFI_WS_PATH);

  _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", WEB_UI_HTML);
  });

  _ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                      void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.printf("WS Client connected: %u\n", client->id());
      _status->setWifiStatus(WIFI_AP_SSID, WiFi.softAPIP().toString().c_str(), _ws->count(), _controllerId != 0);
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.printf("WS Client disconnected: %u\n", client->id());
      if (client->id() == _controllerId) {
        logEvent("WARN", "controller_disconnected");
        revokeController();
      }
      _status->setWifiStatus(WIFI_AP_SSID, WiFi.softAPIP().toString().c_str(), _ws->count(), _controllerId != 0);
    } else if (type == WS_EVT_DATA) {
      handleWebSocketMessage(arg, data, len, client->id());
    }
  });

  _server->addHandler(_ws);
  _server->begin();
}

void WifiControl::startSoftAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
}

void WifiControl::logEvent(const char* severity, const char* code) {
  _eventLog[_eventLogHead].timestampMs = millis();
  _eventLog[_eventLogHead].severity = severity;
  _eventLog[_eventLogHead].code = code;
  _eventLogHead = (_eventLogHead + 1) % EVENT_LOG_SIZE;
}

String WifiControl::generateToken() {
  uint32_t words[4];
  esp_fill_random(words, sizeof(words));
  char buf[33];
  snprintf(buf, sizeof(buf), "%08x%08x%08x%08x", words[0], words[1], words[2], words[3]);
  return String(buf);
}

void WifiControl::generatePairingCode() {
  uint32_t r1 = esp_random() % 10000;
  uint32_t r2 = esp_random() % 10000;
  snprintf(_pairingCode, sizeof(_pairingCode), "%04u %04u", r1, r2);
  _status->setPairingCode(_pairingCode);
  _pairingStartTime = millis();
}

void WifiControl::revokeController() {
  sendStop();
  _controllerId = 0;
  _sessionToken = "";
  _status->setWifiStatus(WIFI_AP_SSID, WiFi.softAPIP().toString().c_str(), _ws ? _ws->count() : 0, false);
}

void WifiControl::sendEvents(uint32_t clientId) {
  size_t count = 0;
  EventLogEntry sorted[EVENT_LOG_SIZE];
  for (size_t i = 0; i < EVENT_LOG_SIZE; i++) {
     size_t idx = (_eventLogHead + i) % EVENT_LOG_SIZE;
     if (_eventLog[idx].timestampMs != 0) {
         sorted[count++] = _eventLog[idx];
     }
  }
  if (count > 0) {
     _ws->text(clientId, _protocol.generateEventLog(sorted, count));
  }
}

void WifiControl::handleWebSocketMessage(void* arg, uint8_t* data, size_t len, uint32_t clientId) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    if (len >= WIFI_MAX_WS_FRAME_BYTES) {
      _ws->text(clientId, _protocol.generateError(0, "payload_too_large"));
      return;
    }
    
    data[len] = 0;
    
    uint32_t now = millis();
    if (now - _lastSecondMs > 1000) {
      _lastSecondMs = now;
      _cmdCountThisSecond = 0;
    }
    _cmdCountThisSecond++;
    if (_cmdCountThisSecond > WIFI_MAX_COMMANDS_PER_SECOND) {
      _ws->text(clientId, _protocol.generateError(0, "rate_limited"));
      return;
    }

    RobotCommand cmd;
    String type, token, code, errCode;
    uint32_t msgId;

    if (_protocol.parseCommand((const char*)data, len, cmd, type, token, code, msgId, errCode)) {
      if (type == "hello" || type == "ping" || type == "status") {
        _ws->text(clientId, _protocol.generateAck(msgId, true, "ok", _router->currentEpoch()));
        sendEvents(clientId);
        return;
      }
      
      if (type == "pair") {
        if (now - _lastMinuteMs > 60000) {
           _lastMinuteMs = now;
           _pairAttemptsThisMinute = 0;
        }
        _pairAttemptsThisMinute++;
        if (_pairAttemptsThisMinute > WIFI_MAX_PAIR_ATTEMPTS_PER_MINUTE) {
           _ws->text(clientId, _protocol.generateError(msgId, "rate_limited"));
           logEvent("WARN", "pair_rate_limit");
           return;
        }

        if (_pairingCode[0] == '\0') {
          _ws->text(clientId, _protocol.generateError(msgId, "pairing_unavailable"));
          return;
        }

        if (code == _pairingCode) {
          _controllerId = clientId;
          _sessionToken = generateToken();
          _leaseExpiryMs = now + WIFI_CONTROLLER_LEASE_MS;
          
          _pairingCode[0] = '\0';
          _status->clearPairingCode();
          _status->setWifiStatus(WIFI_AP_SSID, WiFi.softAPIP().toString().c_str(), _ws->count(), true);
          
          logEvent("INFO", "paired");
          
          JsonDocument ackDoc;
          ackDoc["type"] = "ack";
          if (msgId > 0) ackDoc["id"] = msgId;
          ackDoc["ok"] = true;
          ackDoc["token"] = _sessionToken;
          String ackStr;
          serializeJson(ackDoc, ackStr);
          _ws->text(clientId, ackStr);
          
          sendEvents(clientId);
          return;
        } else {
          _ws->text(clientId, _protocol.generateError(msgId, "invalid_code"));
          logEvent("WARN", "pair_failed");
          return;
        }
      }

      if (token != _sessionToken || _sessionToken == "" || clientId != _controllerId) {
        _ws->text(clientId, _protocol.generateError(msgId, "unauthorized"));
        return;
      }

      _leaseExpiryMs = now + WIFI_CONTROLLER_LEASE_MS;

      portENTER_CRITICAL(&_queueMux);
      size_t next = (_cmdQueueHead + 1) % CMD_QUEUE_SIZE;
      if (next != _cmdQueueTail) {
        _cmdQueue[_cmdQueueHead] = {cmd, now, _router->currentEpoch(), msgId, clientId};
        _cmdQueueHead = next;
      } else {
        _ws->text(clientId, _protocol.generateError(msgId, "queue_full"));
      }
      portEXIT_CRITICAL(&_queueMux);

    } else {
      _ws->text(clientId, _protocol.generateError(msgId, errCode));
    }
  }
}

void WifiControl::sendStop() {
  RobotCommand stopCmd;
  stopCmd.kind = CommandKind::STOP;
  stopCmd.source = ControlSource::WIFI;
  
  portENTER_CRITICAL(&_queueMux);
  size_t next = (_cmdQueueHead + 1) % CMD_QUEUE_SIZE;
  if (next != _cmdQueueTail) {
    _cmdQueue[_cmdQueueHead] = {stopCmd, millis(), _router->currentEpoch(), 0, 0};
    _cmdQueueHead = next;
  }
  portEXIT_CRITICAL(&_queueMux);
}

void WifiControl::update() {
  if (!_server) return;
  
  uint32_t now = millis();

  if (_pairingCode[0] != '\0' && (now - _pairingStartTime > 600000)) {
    _pairingCode[0] = '\0';
    _status->clearPairingCode();
    logEvent("INFO", "pairing_timeout");
  }

  _ws->cleanupClients();

  if (_controllerId != 0) {
    if (now > _leaseExpiryMs) {
      logEvent("WARN", "lease_expired");
      revokeController();
    } else if (_lastDriveCommandMs != 0 && (now - _lastDriveCommandMs > WIFI_DRIVE_WATCHDOG_MS)) {
      sendStop();
      _lastDriveCommandMs = 0;
      logEvent("WARN", "drive_watchdog");
    }
  }

  portENTER_CRITICAL(&_queueMux);
  while (_cmdQueueTail != _cmdQueueHead) {
    QueuedCommand qc = _cmdQueue[_cmdQueueTail];
    _cmdQueueTail = (_cmdQueueTail + 1) % CMD_QUEUE_SIZE;
    portEXIT_CRITICAL(&_queueMux);
    
    bool executeIt = true;
    if (_router->lastInterventionEpoch() > qc.enqueuedEpoch) {
      if (qc.cmd.kind != CommandKind::STOP && qc.cmd.kind != CommandKind::DISARM) {
        executeIt = false;
      }
    }

    bool ok = false;
    String replyMsg = "";
    if (executeIt) {
      if (qc.cmd.kind == CommandKind::ARM && !ALLOW_MOTOR_ARMING) {
        replyMsg = "motors_disabled_in_firmware";
        executeIt = false;
      } else {
        ok = _router->execute(qc.cmd);
        if (qc.cmd.kind == CommandKind::MOVE) {
          _lastDriveCommandMs = millis();
        }
      }
    } else {
      replyMsg = "superseded";
    }

    if (qc.msgId > 0 && qc.clientId > 0) {
      _ws->text(qc.clientId, _protocol.generateAck(qc.msgId, ok, replyMsg, _router->currentEpoch()));
    }
    
    portENTER_CRITICAL(&_queueMux);
  }
  portEXIT_CRITICAL(&_queueMux);

  if (now - _lastTelemetryMs > WIFI_TELEMETRY_INTERVAL_MS) {
    _lastTelemetryMs = now;
    if (_ws->count() > 0) {
      broadcastTelemetry();
    }
  }
}

void WifiControl::broadcastTelemetry() {
  RobotTelemetry t;
  t.revision = _router->currentEpoch();
  t.uptimeMs = millis();
  t.wifiApRunning = WIFI_START_SOFT_AP;
  t.wifiClientCount = _ws->count();
  t.controllerPresent = (_controllerId != 0);
  t.motorsArmed = _robot->isArmed();
  t.motorAllowedByFirmware = ALLOW_MOTOR_ARMING;
  t.driveMode = DriveMode::STOPPED;
  DriveCommand dCmd;
  if (_robot->lastDriveCommand(dCmd)) {
    t.driveMode = dCmd.mode;
  }
  t.action = _robot->currentAction();
  t.mood = _robot->getMood();
  RangeReading r = _robot->rangeReading();
  t.rangeMm = r.distanceMm;
  t.rangeValid = r.valid;
  t.obstacleDetected = _robot->obstacleDetected();
  t.autonomyEnabled = _robot->autonomyEnabled();
  t.buildName = getActiveBuildName();
  t.personaName = DEFAULT_PERSONA;
  t.lastSafetyStopMs = _robot->lastSafetyStopMs();
  t.lastSafetyStopReason = _robot->lastSafetyStopReason();

  String telemetryJson = _protocol.generateTelemetry(t);
  _ws->textAll(telemetryJson);
}
