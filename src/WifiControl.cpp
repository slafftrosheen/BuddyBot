#include "WifiControl.h"
#include "WebUiPage.h"
#include "arduino_secrets.h"
#include "BuildProfiles.h"

void WifiControl::begin(ControlRouter* router, RobotAPI* robot, SystemStatus* status) {
  _router = router;
  _robot = robot;
  _status = status;
}

bool WifiControl::start() {
  if (!ENABLE_WIFI_CONTROL) return false;
  if (_running) return true;

  if (WIFI_START_SOFT_AP) {
    if (!startSoftAP()) {
      logEvent("ERROR", "wifi_ap_start_failed");
      return false;
    }
  }
  
  char ipBuf[32];
  strlcpy(ipBuf, WiFi.softAPIP().toString().c_str(), sizeof(ipBuf));
  strlcpy(_apSsid, WIFI_AP_SSID, sizeof(_apSsid));
  strlcpy(_apIp, ipBuf, sizeof(_apIp));
  _status->setWifiStatus(true, _apSsid, _apIp, 0, false, false);
  
  generatePairingCode();
  logEvent("INFO", "wifi_started");

  _server = new AsyncWebServer(WIFI_HTTP_PORT);
  _ws = new AsyncWebSocket(WIFI_WS_PATH);

  _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", WEB_UI_HTML);
    response->addHeader("Cache-Control", "no-store");
    response->addHeader("X-Content-Type-Options", "nosniff");
    response->addHeader("Content-Security-Policy", "default-src 'self'; connect-src 'self' ws:; style-src 'unsafe-inline'; script-src 'unsafe-inline'; img-src 'self' data:; base-uri 'none'; frame-ancestors 'none'");
    request->send(response);
  });
  
  _server->on("/health", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "ok");
  });
  
  _server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(204);
  });

  _ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      WifiClientState* state = nullptr;
      for (size_t i = 0; i < WIFI_MAX_CLIENTS; i++) {
        if (!_clients[i].connected) {
          state = &_clients[i];
          break;
        }
      }
      if (state) {
        state->clientId = client->id();
        state->connected = true;
        state->commandWindowStartedMs = millis();
        state->commandsInWindow = 0;
        state->pairWindowStartedMs = millis();
        state->failedPairAttempts = 0;
        _status->setWifiStatus(true, _apSsid, _apIp, _ws->count(), _session.active, pairingAvailable());
      } else {
        client->close();
      }
    } else if (type == WS_EVT_DISCONNECT) {
      if (client->id() == _session.clientId) {
        logEvent("WARN", "controller_disconnected");
        revokeController();
      }
      for (size_t i = 0; i < WIFI_MAX_CLIENTS; i++) {
        if (_clients[i].connected && _clients[i].clientId == client->id()) {
          _clients[i].connected = false;
        }
      }
      _status->setWifiStatus(true, _apSsid, _apIp, _ws ? _ws->count() : 0, _session.active, pairingAvailable());
    } else if (type == WS_EVT_DATA) {
      handleWebSocketMessage(arg, data, len, client->id());
    }
  });

  _server->addHandler(_ws);
  _server->begin();
  _running = true;
  return true;
}

void WifiControl::stop() {
  if (!_running) return;
  revokeController();
  
  if (_ws) {
    _ws->closeAll();
    delete _ws;
    _ws = nullptr;
  }
  if (_server) {
    _server->end();
    delete _server;
    _server = nullptr;
  }
  WiFi.softAPdisconnect(true);
  
  _pairingCode[0] = '\0';
  _status->clearPairingCode();
  _status->setWifiStatus(false, "", "", 0, false, false);
  
  logEvent("INFO", "wifi_stopped");
  _running = false;
}

bool WifiControl::running() const { return _running; }
bool WifiControl::controllerPresent() const { return _session.active; }
bool WifiControl::pairingAvailable() const { return _pairingCode[0] != '\0'; }

bool WifiControl::requestNewPairingCode() {
  if (!_running || controllerPresent()) return false;
  generatePairingCode();
  return true;
}

uint8_t WifiControl::clientCount() const { return _ws ? _ws->count() : 0; }
const char* WifiControl::apSsid() const { return _apSsid; }
const char* WifiControl::apIp() const { return _apIp; }

bool WifiControl::startSoftAP() {
  if (WIFI_AP_SSID[0] == '\0' || WIFI_AP_PASSWORD[0] == '\0') {
    return false;
  }
  WiFi.mode(WIFI_AP);
  return WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
}

void WifiControl::syncStatus() {
  if (_status) {
    _status->setWifiStatus(_running, _apSsid, _apIp, clientCount(), _session.active, pairingAvailable());
  }
}

void WifiControl::logEvent(const char* severity, const char* code) {
  EventLog::instance().log(severity, code);
  broadcastEvents();
}

void WifiControl::generateToken(char* outBuffer) {
  uint32_t words[4];
  esp_fill_random(words, sizeof(words));
  snprintf(outBuffer, WIFI_SESSION_TOKEN_HEX_CHARS + 1, "%08x%08x%08x%08x", words[0], words[1], words[2], words[3]);
}

void WifiControl::generatePairingCode() {
  uint32_t r1 = esp_random() % 10000;
  uint32_t r2 = esp_random() % 10000;
  snprintf(_pairingCode, sizeof(_pairingCode), "%04u%04u", r1, r2);
  
  char displayBuf[16];
  snprintf(displayBuf, sizeof(displayBuf), "%04u %04u", r1, r2);
  _status->setPairingCode(displayBuf);
  _pairingStartTime = millis();
  _status->setWifiStatus(true, _apSsid, _apIp, _ws ? _ws->count() : 0, _session.active, true);
  broadcastTelemetry();
}

void WifiControl::revokeController() {
  if (_session.active) {
    requestEmergencyStopFromWifi(_session.clientId);
    _session.active = false;
    _session.token[0] = '\0';
    _session.clientId = 0;
    
    _status->setWifiStatus(true, _apSsid, _apIp, _ws ? _ws->count() : 0, false, pairingAvailable());
    
    generatePairingCode();
    broadcastTelemetry();
  }
}

void WifiControl::sendEvents(uint32_t clientId) {
  EventLogEntry sorted[EventLog::CAPACITY];
  size_t count = EventLog::instance().getEntries(sorted, EventLog::CAPACITY);
  if (count > 0) {
     _ws->text(clientId, _protocol.generateEventLog(sorted, count));
  }
}

WifiClientState* WifiControl::getClientState(uint32_t clientId) {
  for (size_t i = 0; i < WIFI_MAX_CLIENTS; i++) {
    if (_clients[i].connected && _clients[i].clientId == clientId) return &_clients[i];
  }
  return nullptr;
}

bool WifiControl::constantTimeEquals(const char* a, const char* b, size_t n) {
  uint8_t diff = 0;
  for (size_t i = 0; i < n; i++) {
    diff |= (a[i] ^ b[i]);
  }
  return diff == 0;
}

bool WifiControl::enqueueCommand(const QueuedCommand& cmd) {
  bool success = false;
  portENTER_CRITICAL(&_queueMux);
  if (_cmdQueueSize < WIFI_COMMAND_QUEUE_CAPACITY) {
    _cmdQueue[_cmdQueueTail] = cmd;
    _cmdQueueTail = (_cmdQueueTail + 1) % WIFI_COMMAND_QUEUE_CAPACITY;
    _cmdQueueSize++;
    success = true;
  }
  portEXIT_CRITICAL(&_queueMux);
  return success;
}

bool WifiControl::dequeueCommand(QueuedCommand& cmd) {
  bool success = false;
  portENTER_CRITICAL(&_queueMux);
  if (_cmdQueueSize > 0) {
    cmd = _cmdQueue[_cmdQueueHead];
    _cmdQueueHead = (_cmdQueueHead + 1) % WIFI_COMMAND_QUEUE_CAPACITY;
    _cmdQueueSize--;
    success = true;
  }
  portEXIT_CRITICAL(&_queueMux);
  return success;
}

void WifiControl::requestEmergencyStopFromWifi(uint32_t clientId) {
  portENTER_CRITICAL(&_queueMux);
  _pendingStop = true;
  _pendingStopClientId = clientId;
  portEXIT_CRITICAL(&_queueMux);
}

void WifiControl::handleWebSocketMessage(void* arg, uint8_t* data, size_t len, uint32_t clientId) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
  if (info->opcode != WS_TEXT) {
    _ws->close(clientId, 1003, "binary_not_supported");
    return;
  }
  
  if (!info->final || info->index != 0 || info->len != len) {
    _ws->close(clientId, 1003, "fragmented_not_supported");
    return;
  }

  WifiClientState* state = getClientState(clientId);
  if (!state) return;

  WebParsedMessage msg;
  WebProtocolError err;
  if (!_protocol.parseCommand(data, len, msg, err)) {
    _ws->text(clientId, _protocol.generateError(msg.requestId, _protocol.webProtocolErrorName(err)));
    return;
  }

  uint32_t now = millis();
  
  if (msg.type != WebMessageType::STOP) {
    if (now - state->commandWindowStartedMs > 1000) {
      state->commandWindowStartedMs = now;
      state->commandsInWindow = 0;
    }
    state->commandsInWindow++;
    if (state->commandsInWindow > WIFI_MAX_COMMANDS_PER_SECOND) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "rate_limited"));
      return;
    }
  }

  if (msg.type == WebMessageType::HELLO || msg.type == WebMessageType::PING || msg.type == WebMessageType::STATUS) {
    if (_session.active && _session.clientId == clientId) {
      _session.leaseExpiryMs = now + WIFI_CONTROLLER_LEASE_MS;
    }
    _ws->text(clientId, _protocol.generateAck(msg.requestId, true, "ok", _router->currentEpoch()));
    sendEvents(clientId);
    return;
  }

  if (msg.type == WebMessageType::PAIR) {
    if (now - state->pairWindowStartedMs > 60000) {
       state->pairWindowStartedMs = now;
       state->failedPairAttempts = 0;
    }
    state->failedPairAttempts++;
    if (state->failedPairAttempts > WIFI_MAX_PAIR_ATTEMPTS_PER_MINUTE) {
       _ws->text(clientId, _protocol.generateError(msg.requestId, "rate_limited"));
       logEvent("WARN", "pair_rate_limit");
       return;
    }

    if (_session.active && _session.clientId != clientId) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "controller_busy"));
      return;
    }

    if (!pairingAvailable()) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "pairing_unavailable"));
      return;
    }

    if (constantTimeEquals(msg.pairingDigits, _pairingCode, WIFI_PAIRING_DIGITS)) {
      state->failedPairAttempts = 0;
      _session.active = true;
      _session.clientId = clientId;
      generateToken(_session.token);
      _session.leaseExpiryMs = now + WIFI_CONTROLLER_LEASE_MS;
      _session.lastAcceptedCommandMs = now;
      _session.lastActiveDriveMs = 0;
      _session.driveWatchdogStopped = false;
      
      _pairingCode[0] = '\0';
      _status->clearPairingCode();
      _status->setWifiStatus(true, _apSsid, _apIp, _ws->count(), true, false);
      
      logEvent("INFO", "paired");
      
      JsonDocument ackDoc;
      ackDoc["type"] = "paired";
      if (msg.hasRequestId) ackDoc["id"] = msg.requestId;
      ackDoc["ok"] = true;
      ackDoc["token"] = _session.token;
      ackDoc["leaseMs"] = WIFI_CONTROLLER_LEASE_MS;
      String ackStr;
      serializeJson(ackDoc, ackStr);
      _ws->text(clientId, ackStr);
      
      broadcastTelemetry();
      sendEvents(clientId);
      return;
    } else {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "pairing_failed"));
      logEvent("WARN", "pair_failed");
      return;
    }
  }

  if (msg.type == WebMessageType::STOP) {
    if ((_session.active && _session.clientId == clientId) && 
        (!msg.hasToken || constantTimeEquals(msg.token, _session.token, WIFI_SESSION_TOKEN_HEX_CHARS))) {
      requestEmergencyStopFromWifi(clientId);
      _session.leaseExpiryMs = now + WIFI_CONTROLLER_LEASE_MS;
      _session.lastAcceptedCommandMs = now;
      if (msg.hasRequestId) {
        _ws->text(clientId, _protocol.generateAck(msg.requestId, true, "ok", _router->currentEpoch()));
      }
      return;
    }
    if (msg.hasToken) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "bad_token"));
    } else {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "not_controller"));
    }
    return;
  }

  if (!_session.active || _session.clientId != clientId) {
    _ws->text(clientId, _protocol.generateError(msg.requestId, "not_controller"));
    return;
  }
  
  if (!msg.hasToken || !constantTimeEquals(msg.token, _session.token, WIFI_SESSION_TOKEN_HEX_CHARS)) {
    _ws->text(clientId, _protocol.generateError(msg.requestId, "bad_token"));
    return;
  }

  QueuedCommand qc;
  qc.command = msg.command;
  qc.enqueuedMs = now;
  qc.observedEpoch = _router->currentEpoch();
  qc.requestId = msg.hasRequestId ? msg.requestId : 0;
  qc.clientId = clientId;
  
  if (enqueueCommand(qc)) {
    _session.leaseExpiryMs = now + WIFI_CONTROLLER_LEASE_MS;
    _session.lastAcceptedCommandMs = now;
  } else {
    _ws->text(clientId, _protocol.generateError(msg.requestId, "queue_full"));
  }
}

void WifiControl::update() {
  if (!_running) return;
  
  uint32_t now = millis();

  if (pairingAvailable() && (now - _pairingStartTime > WIFI_PAIRING_CODE_LIFETIME_MS)) {
    _pairingCode[0] = '\0';
    _status->clearPairingCode();
    _status->setWifiStatus(true, _apSsid, _apIp, _ws ? _ws->count() : 0, _session.active, false);
    broadcastTelemetry();
  }

  if (_ws) _ws->cleanupClients();
  syncStatus();

  if (_session.active) {
    if (now > _session.leaseExpiryMs) {
      logEvent("WARN", "lease_expired");
      revokeController();
    } else if (!_session.driveWatchdogStopped && _session.lastActiveDriveMs != 0 && (now - _session.lastActiveDriveMs > WIFI_DRIVE_WATCHDOG_MS)) {
      requestEmergencyStopFromWifi(_session.clientId);
      _session.driveWatchdogStopped = true;
      _session.lastActiveDriveMs = 0;
      logEvent("WARN", "drive_watchdog");
    }
  }

  bool doPendingStop = false;
  uint32_t stopClientId = 0;
  portENTER_CRITICAL(&_queueMux);
  if (_pendingStop) {
    doPendingStop = true;
    stopClientId = _pendingStopClientId;
    _pendingStop = false;
  }
  portEXIT_CRITICAL(&_queueMux);

  if (doPendingStop) {
    RobotCommand stopCmd;
    stopCmd.kind = CommandKind::STOP;
    stopCmd.source = ControlSource::WIFI;
    _router->execute(stopCmd);
    _session.driveWatchdogStopped = true;
    _session.lastActiveDriveMs = 0;
    broadcastTelemetry();
  }

  QueuedCommand qc;
  while (dequeueCommand(qc)) {
    bool executeIt = true;
    if (_router->lastInterventionEpoch() > qc.observedEpoch) {
      if (qc.command.kind != CommandKind::STOP && qc.command.kind != CommandKind::DISARM) {
        executeIt = false;
      }
    }

    bool ok = false;
    String replyMsg = "";
    if (executeIt) {
      if (qc.command.kind == CommandKind::ARM && !ALLOW_MOTOR_ARMING) {
        replyMsg = "motors_locked";
        executeIt = false;
      } else {
        ok = _router->execute(qc.command);
        if (!ok && qc.command.kind == CommandKind::MOVE) {
          if (_robot) {
            SafetyStopReason reason = _robot->lastSafetyStopReason();
            if (reason == SafetyStopReason::OBSTACLE_BLOCKED) {
              replyMsg = "obstacle_blocked";
            } else if (reason == SafetyStopReason::RANGE_SENSOR_INVALID || reason == SafetyStopReason::RANGE_SENSOR_STALE) {
              replyMsg = "sensor_unavailable";
            }
          }
        }
        if (qc.command.kind == CommandKind::MOVE) {
          _session.lastActiveDriveMs = millis();
          _session.driveWatchdogStopped = false;
        }
      }
    } else {
      replyMsg = "superseded";
    }

    if (qc.requestId > 0 && qc.clientId > 0) {
      _ws->text(qc.clientId, _protocol.generateAck(qc.requestId, ok, replyMsg, _router->currentEpoch()));
    }
    
    if (executeIt) broadcastTelemetry();
  }

  if (now - _lastTelemetryMs > WIFI_TELEMETRY_INTERVAL_MS) {
    _lastTelemetryMs = now;
    if (_ws && _ws->count() > 0) {
      broadcastTelemetry();
    }
  }
}
void WifiControl::broadcastTelemetry() {
  RobotTelemetry t;
  t.revision = _router->currentEpoch();
  t.uptimeMs = millis();
  t.wifiApRunning = _running && WIFI_START_SOFT_AP;
  t.wifiClientCount = _ws ? _ws->count() : 0;
  t.controllerPresent = _session.active;
  t.pairingAvailable = pairingAvailable();
  
  if (_robot) {
    ActuatorCapabilities caps = _robot->actuatorCapabilities();
    t.hasDrive = caps.driveAvailable;
    t.fourWheelDrive = caps.fourWheelDrive;
    t.hasManipulators = caps.manipulatorAvailable;
    
    t.motorsArmed = _robot->isArmed();
    t.motorAllowedByFirmware = ALLOW_MOTOR_ARMING;
  }
  
  t.driveMode = DriveMode::STOPPED;
  t.driveModeName = "stop";
  DriveCommand dCmd;
  if (_robot->lastDriveCommand(dCmd)) {
    t.driveMode = dCmd.mode;
    if (t.driveMode == DriveMode::FORWARD) t.driveModeName = "forward";
    else if (t.driveMode == DriveMode::REVERSE) t.driveModeName = "reverse";
    else if (t.driveMode == DriveMode::TURN_LEFT) t.driveModeName = "turn_left";
    else if (t.driveMode == DriveMode::TURN_RIGHT) t.driveModeName = "turn_right";
  }
  
  t.action = _robot->currentAction();
  t.actionRunning = (t.action != ActionId::NONE);
  switch (t.action) {
    case ActionId::WAVE: t.actionName = "wave"; break;
    case ActionId::LOOK_LEFT: t.actionName = "look_left"; break;
    case ActionId::LOOK_RIGHT: t.actionName = "look_right"; break;
    case ActionId::GREET: t.actionName = "greet"; break;
    case ActionId::CELEBRATE: t.actionName = "celebrate"; break;
    case ActionId::DANCE: t.actionName = "dance"; break;
    case ActionId::SLEEP: t.actionName = "sleep"; break;
    default: t.actionName = "none"; break;
  }
  
  t.mood = _robot->getMood();
  switch (t.mood) {
    case Mood::HAPPY: t.moodName = "happy"; break;
    case Mood::CURIOUS: t.moodName = "curious"; break;
    case Mood::SLEEPY: t.moodName = "sleepy"; break;
    case Mood::EXCITED: t.moodName = "excited"; break;
    case Mood::ALERT: t.moodName = "alert"; break;
    default: t.moodName = "idle"; break;
  }
  
  RangeReading r = _robot->rangeReading();
  t.rangeMm = r.distanceMm;
  t.rangeValid = r.valid;
  t.obstacleDetected = _robot->obstacleDetected();
  t.autonomyEnabled = _robot->autonomyEnabled();
  
  t.autonomyMode = t.autonomyEnabled ? "ASSISTED_AVOIDANCE" : "OFF";
  t.autonomyState = "IDLE"; // AutonomyManager state is internal, mock it for now or expose it

  t.buildName = getActiveBuildName();
  t.personaName = _robot->personaName();
  
  FirmwareIdentity id = getFirmwareIdentity();
  t.firmwareVersion = id.version;
  t.firmwareChannel = id.channel;
  
  const ObstacleSafetyStatus& st = _robot->obstacleSafetyStatus();
  t.lastSafetyStopMs = st.lastStopMs;
  switch (st.lastStopReason) {
    case SafetyStopReason::NONE: t.lastSafetyStopReason = "none"; break;
    case SafetyStopReason::OBSTACLE_BLOCKED: t.lastSafetyStopReason = "obstacle_blocked"; break;
    case SafetyStopReason::RANGE_SENSOR_STALE: t.lastSafetyStopReason = "range_sensor_stale"; break;
    case SafetyStopReason::RANGE_SENSOR_INVALID: t.lastSafetyStopReason = "range_sensor_invalid"; break;
    case SafetyStopReason::AUTONOMY_TIMEOUT: t.lastSafetyStopReason = "autonomy_timeout"; break;
    case SafetyStopReason::MANUAL_STOP: t.lastSafetyStopReason = "manual_stop"; break;
    case SafetyStopReason::CONTROLLER_DISCONNECT: t.lastSafetyStopReason = "controller_disconnect"; break;
    case SafetyStopReason::CONTROLLER_LEASE_EXPIRED: t.lastSafetyStopReason = "controller_lease_expired"; break;
    case SafetyStopReason::DRIVE_WATCHDOG: t.lastSafetyStopReason = "drive_watchdog"; break;
    case SafetyStopReason::DISARMED: t.lastSafetyStopReason = "disarmed"; break;
    default: t.lastSafetyStopReason = "unknown"; break;
  }
  
  switch (st.state) {
    case ObstacleSafetyState::CLEAR: t.obstacleSafetyState = "clear"; break;
    case ObstacleSafetyState::CAUTION: t.obstacleSafetyState = "caution"; break;
    case ObstacleSafetyState::BLOCKED: t.obstacleSafetyState = "blocked"; break;
    case ObstacleSafetyState::SENSOR_UNAVAILABLE: t.obstacleSafetyState = "sensor_unavailable"; break;
  }
  
  RangeSensorHealth health = _robot->rangeSensorHealth();
  switch (health) {
    case RangeSensorHealth::UNINITIALIZED: t.rangeSensorHealth = "uninitialized"; break;
    case RangeSensorHealth::READY: t.rangeSensorHealth = "ready"; break;
    case RangeSensorHealth::STALE: t.rangeSensorHealth = "stale"; break;
    case RangeSensorHealth::INVALID: t.rangeSensorHealth = "invalid"; break;
    case RangeSensorHealth::UNAVAILABLE: t.rangeSensorHealth = "unavailable"; break;
  }
  
  t.forwardMotionBlocked = st.forwardMotionBlocked;

  String telemetryJson = _protocol.generateTelemetry(t);
  if (_ws) _ws->textAll(telemetryJson);
}

void WifiControl::broadcastEvents() {
  if (!_running || !_ws) return;
  EventLogEntry entries[EventLog::CAPACITY];
  size_t count = EventLog::instance().getEntries(entries, EventLog::CAPACITY);
  if (count > 0) {
    String eventsJson = _protocol.generateEventLog(entries, count);
    _ws->textAll(eventsJson);
  }
}
