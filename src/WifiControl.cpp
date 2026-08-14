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
  if (_dispatchMutex == nullptr) {
    _dispatchMutex = xSemaphoreCreateMutex();
    if (_dispatchMutex == nullptr) {
      logEvent("ERROR", "wifi_dispatch_lock_failed");
      return false;
    }
  }

  portENTER_CRITICAL(&_stateMux);
  _session = {};
  for (size_t i = 0; i < WIFI_MAX_CLIENTS; ++i) {
    _clients[i] = {};
  }
  _globalPairWindowStartedMs = millis();
  _globalFailedPairAttempts = 0;
  _pairingLockedUntilMs = 0;
  _nextSessionGeneration = 0;
  portEXIT_CRITICAL(&_stateMux);
  portENTER_CRITICAL(&_queueMux);
  _cmdQueueHead = 0;
  _cmdQueueTail = 0;
  _cmdQueueSize = 0;
  _pendingStop = false;
  _pendingStopClientId = 0;
  _pendingStopFault = SafetyFault::DRIVE_WATCHDOG;
  portEXIT_CRITICAL(&_queueMux);

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
      bool sessionActive = false;
      bool pairingReady = false;
      const uint32_t now = millis();
      portENTER_CRITICAL(&_stateMux);
      for (size_t i = 0; i < WIFI_MAX_CLIENTS; i++) {
        if (!_clients[i].connected) {
          state = &_clients[i];
          break;
        }
      }
      if (state) {
        state->clientId = client->id();
        state->connected = true;
        state->connectedSinceMs = now;
        state->lastActivityMs = now;
        state->commandWindowStartedMs = now;
        state->commandsInWindow = 0;
        state->pairWindowStartedMs = now;
        state->failedPairAttempts = 0;
        sessionActive = _session.active;
        pairingReady = _pairingCode[0] != '\0';
      }
      portEXIT_CRITICAL(&_stateMux);

      if (state) {
        _status->setWifiStatus(true, _apSsid, _apIp, _ws->count(), sessionActive, pairingReady);
      } else {
        client->close();
      }
    } else if (type == WS_EVT_DISCONNECT) {
      bool wasController = false;
      portENTER_CRITICAL(&_stateMux);
      wasController = _session.active && client->id() == _session.clientId;
      for (size_t i = 0; i < WIFI_MAX_CLIENTS; i++) {
        if (_clients[i].connected && _clients[i].clientId == client->id()) {
          _clients[i] = {};
        }
      }
      portEXIT_CRITICAL(&_stateMux);

      if (wasController) {
        logEvent("WARN", "controller_disconnected");
        revokeController(SafetyFault::CONTROLLER_DISCONNECTED);
      }
      syncStatus();
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
  const bool hadController = controllerPresent();
  revokeController();
  if (hadController && _router) {
    _router->emergencyStop(SafetyFault::CONTROLLER_DISCONNECTED);
    portENTER_CRITICAL(&_queueMux);
    _pendingStop = false;
    portEXIT_CRITICAL(&_queueMux);
  }
  
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
  
  portENTER_CRITICAL(&_stateMux);
  _pairingCode[0] = '\0';
  portEXIT_CRITICAL(&_stateMux);
  _status->clearPairingCode();
  _status->setWifiStatus(false, "", "", 0, false, false);
  
  logEvent("INFO", "wifi_stopped");
  _running = false;
}

bool WifiControl::running() const { return _running; }
bool WifiControl::controllerPresent() const {
  portENTER_CRITICAL(&_stateMux);
  const bool active = _session.active;
  portEXIT_CRITICAL(&_stateMux);
  return active;
}

bool WifiControl::pairingAvailable() const {
  portENTER_CRITICAL(&_stateMux);
  const bool available = _pairingCode[0] != '\0';
  portEXIT_CRITICAL(&_stateMux);
  return available;
}

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
    bool sessionActive = false;
    bool pairingReady = false;
    portENTER_CRITICAL(&_stateMux);
    sessionActive = _session.active;
    pairingReady = _pairingCode[0] != '\0';
    portEXIT_CRITICAL(&_stateMux);
    _status->setWifiStatus(_running, _apSsid, _apIp, clientCount(), sessionActive, pairingReady);
  }
}

void WifiControl::logEvent(const char* severity, const char* code) {
  EventLog::instance().log(severity, code, "wifi");
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
  char pairingCode[WIFI_PAIRING_DIGITS + 1] = {};
  snprintf(pairingCode, sizeof(pairingCode), "%04u%04u", r1, r2);
  
  char displayBuf[16];
  snprintf(displayBuf, sizeof(displayBuf), "%04u %04u", r1, r2);

  bool sessionActive = false;
  portENTER_CRITICAL(&_stateMux);
  strlcpy(_pairingCode, pairingCode, sizeof(_pairingCode));
  _pairingStartTime = millis();
  sessionActive = _session.active;
  portEXIT_CRITICAL(&_stateMux);

  _status->setPairingCode(displayBuf);
  _status->setWifiStatus(true, _apSsid, _apIp, _ws ? _ws->count() : 0, sessionActive, true);
  broadcastTelemetry();
}

void WifiControl::revokeController(SafetyFault fault) {
  uint32_t clientId = 0;
  bool wasActive = false;
  if (!acquireDispatchLock()) {
    return;
  }
  portENTER_CRITICAL(&_stateMux);
  if (_session.active) {
    clientId = _session.clientId;
    _session = {};
    wasActive = true;
  }
  portEXIT_CRITICAL(&_stateMux);

  if (!wasActive) {
    releaseDispatchLock();
    return;
  }

  clearQueuedCommands();
  requestEmergencyStopFromWifi(clientId, fault);
  releaseDispatchLock();
  generatePairingCode();
  broadcastTelemetry();
}

void WifiControl::sendEvents(uint32_t clientId) {
  EventLogEntry sorted[EventLog::CAPACITY];
  size_t count = EventLog::instance().getEntries(sorted, EventLog::CAPACITY);
  if (count > 0) {
     _ws->text(clientId, _protocol.generateEventLog(sorted, count));
  }
}

WifiClientState* WifiControl::getClientState(uint32_t clientId) {
  portENTER_CRITICAL(&_stateMux);
  WifiClientState* result = nullptr;
  for (size_t i = 0; i < WIFI_MAX_CLIENTS; i++) {
    if (_clients[i].connected && _clients[i].clientId == clientId) {
      result = &_clients[i];
      break;
    }
  }
  portEXIT_CRITICAL(&_stateMux);
  return result;
}

bool WifiControl::constantTimeEquals(const char* a, const char* b, size_t n) const {
  uint8_t diff = 0;
  for (size_t i = 0; i < n; i++) {
    diff |= (a[i] ^ b[i]);
  }
  return diff == 0;
}

bool WifiControl::controllerMatches(
  uint32_t clientId,
  const WebParsedMessage& message,
  uint32_t* sessionGeneration
) const {
  if (!message.hasToken) {
    return false;
  }

  portENTER_CRITICAL(&_stateMux);
  const bool matches = _session.active &&
                       _session.clientId == clientId &&
                       constantTimeEquals(message.token, _session.token, WIFI_SESSION_TOKEN_HEX_CHARS);
  if (matches && sessionGeneration) {
    *sessionGeneration = _session.generation;
  }
  portEXIT_CRITICAL(&_stateMux);
  return matches;
}

bool WifiControl::acceptRequestId(uint32_t requestId, uint32_t sessionGeneration) {
  if (requestId == 0) {
    return false;
  }

  portENTER_CRITICAL(&_stateMux);
  const bool accepted = _session.active &&
    _session.generation == sessionGeneration &&
    (!_session.hasLastRequestId ||
     static_cast<int32_t>(requestId - _session.lastRequestId) > 0);
  if (accepted) {
    _session.lastRequestId = requestId;
    _session.hasLastRequestId = true;
  }
  portEXIT_CRITICAL(&_stateMux);
  return accepted;
}

bool WifiControl::commandSessionCurrent(const QueuedCommand& command) const {
  portENTER_CRITICAL(&_stateMux);
  const bool current = _session.active &&
                       _session.clientId == command.clientId &&
                       _session.generation == command.sessionGeneration;
  portEXIT_CRITICAL(&_stateMux);
  return current;
}

void WifiControl::refreshControllerLease(
  uint32_t clientId,
  uint32_t sessionGeneration,
  uint32_t nowMs
) {
  portENTER_CRITICAL(&_stateMux);
  if (_session.active &&
      _session.clientId == clientId &&
      _session.generation == sessionGeneration) {
    _session.leaseExpiryMs = nowMs + WIFI_CONTROLLER_LEASE_MS;
    _session.lastAcceptedCommandMs = nowMs;
  }
  portEXIT_CRITICAL(&_stateMux);
}

bool WifiControl::acquireDispatchLock() {
  return _dispatchMutex != nullptr &&
    xSemaphoreTake(_dispatchMutex, portMAX_DELAY) == pdTRUE;
}

void WifiControl::releaseDispatchLock() {
  if (_dispatchMutex != nullptr) {
    xSemaphoreGive(_dispatchMutex);
  }
}

void WifiControl::recordPairFailure(uint32_t nowMs) {
  portENTER_CRITICAL(&_stateMux);
  if (nowMs - _globalPairWindowStartedMs >= WIFI_PAIRING_LOCKOUT_MS) {
    _globalPairWindowStartedMs = nowMs;
    _globalFailedPairAttempts = 0;
  }
  ++_globalFailedPairAttempts;
  if (_globalFailedPairAttempts > WIFI_MAX_PAIR_ATTEMPTS_PER_MINUTE) {
    _pairingLockedUntilMs = nowMs + WIFI_PAIRING_LOCKOUT_MS;
    _globalPairWindowStartedMs = nowMs;
    _globalFailedPairAttempts = 0;
  }
  portEXIT_CRITICAL(&_stateMux);
}

bool WifiControl::pairingRateLimited(uint32_t nowMs) const {
  portENTER_CRITICAL(&_stateMux);
  const bool limited = _pairingLockedUntilMs != 0 &&
    static_cast<int32_t>(nowMs - _pairingLockedUntilMs) < 0;
  portEXIT_CRITICAL(&_stateMux);
  return limited;
}

void WifiControl::evictIdleUnpairedClients(uint32_t nowMs) {
  uint32_t clientsToClose[WIFI_MAX_CLIENTS] = {};
  size_t closeCount = 0;

  portENTER_CRITICAL(&_stateMux);
  for (size_t i = 0; i < WIFI_MAX_CLIENTS; ++i) {
    const WifiClientState& client = _clients[i];
    if (client.connected &&
        (!_session.active || _session.clientId != client.clientId) &&
        nowMs - client.lastActivityMs >= WIFI_UNPAIRED_CLIENT_TIMEOUT_MS) {
      clientsToClose[closeCount++] = client.clientId;
      _clients[i] = {};
    }
  }
  portEXIT_CRITICAL(&_stateMux);

  for (size_t i = 0; i < closeCount; ++i) {
    if (_ws) {
      _ws->close(clientsToClose[i], 1008, "pairing_timeout");
    }
  }
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

void WifiControl::clearQueuedCommands() {
  portENTER_CRITICAL(&_queueMux);
  _cmdQueueHead = 0;
  _cmdQueueTail = 0;
  _cmdQueueSize = 0;
  portEXIT_CRITICAL(&_queueMux);
}

void WifiControl::requestEmergencyStopFromWifi(uint32_t clientId, SafetyFault fault) {
  portENTER_CRITICAL(&_queueMux);
  if (!_pendingStop ||
      _pendingStopFault == SafetyFault::NONE ||
      fault != SafetyFault::NONE) {
    _pendingStop = true;
    _pendingStopClientId = clientId;
    _pendingStopFault = fault;
  }
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
  bool knownClient = false;
  bool commandRateLimited = false;
  portENTER_CRITICAL(&_stateMux);
  if (state->connected && state->clientId == clientId) {
    knownClient = true;
    state->lastActivityMs = now;
    if (msg.type != WebMessageType::STOP) {
      if (now - state->commandWindowStartedMs >= 1000) {
        state->commandWindowStartedMs = now;
        state->commandsInWindow = 0;
      }
      ++state->commandsInWindow;
      commandRateLimited = state->commandsInWindow > WIFI_MAX_COMMANDS_PER_SECOND;
    }
  }
  portEXIT_CRITICAL(&_stateMux);

  if (!knownClient) {
    return;
  }
  if (commandRateLimited) {
    _ws->text(clientId, _protocol.generateError(msg.requestId, "rate_limited"));
    return;
  }

  if (msg.type == WebMessageType::HELLO || msg.type == WebMessageType::STATUS ||
      msg.type == WebMessageType::PING || msg.type == WebMessageType::HANDSHAKE) {
    if ((msg.type == WebMessageType::PING || msg.type == WebMessageType::HANDSHAKE) && msg.hasToken) {
      uint32_t sessionGeneration = 0;
      if (!msg.hasRequestId || !controllerMatches(clientId, msg, &sessionGeneration)) {
        _ws->text(clientId, _protocol.generateError(msg.requestId, "bad_token"));
        return;
      }
      if (!acceptRequestId(msg.requestId, sessionGeneration)) {
        _ws->text(clientId, _protocol.generateError(msg.requestId, "replayed_command"));
        return;
      }

      refreshControllerLease(clientId, sessionGeneration, now);
    }
    _ws->text(clientId, _protocol.generateAck(msg.requestId, true, "ok", _router->currentEpoch()));
    sendEvents(clientId);
    return;
  }

  if (msg.type == WebMessageType::STATE) {
    RuntimeSnapshot snapshot = _robot->runtimeSnapshot();
    _ws->text(clientId, _protocol.generateRuntimeSnapshot(msg.requestId, snapshot));
    return;
  }

  if (msg.type == WebMessageType::PAIR) {
    if (pairingRateLimited(now)) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "rate_limited"));
      return;
    }

    bool clientRateLimited = false;
    portENTER_CRITICAL(&_stateMux);
    if (now - state->pairWindowStartedMs >= WIFI_PAIRING_LOCKOUT_MS) {
      state->pairWindowStartedMs = now;
      state->failedPairAttempts = 0;
    }
    ++state->failedPairAttempts;
    clientRateLimited = state->failedPairAttempts > WIFI_MAX_PAIR_ATTEMPTS_PER_MINUTE;
    portEXIT_CRITICAL(&_stateMux);
    if (clientRateLimited) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "rate_limited"));
      logEvent("WARN", "pair_rate_limit");
      return;
    }

    char pairingCode[WIFI_PAIRING_DIGITS + 1] = {};
    bool controllerBusy = false;
    bool pairingReady = false;
    portENTER_CRITICAL(&_stateMux);
    controllerBusy = _session.active && _session.clientId != clientId;
    pairingReady = _pairingCode[0] != '\0';
    strlcpy(pairingCode, _pairingCode, sizeof(pairingCode));
    portEXIT_CRITICAL(&_stateMux);

    if (controllerBusy) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "controller_busy"));
      return;
    }
    if (!pairingReady) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "pairing_unavailable"));
      return;
    }

    if (!constantTimeEquals(msg.pairingDigits, pairingCode, WIFI_PAIRING_DIGITS)) {
      recordPairFailure(now);
      _ws->text(clientId, _protocol.generateError(
        msg.requestId,
        pairingRateLimited(now) ? "rate_limited" : "pairing_failed"
      ));
      logEvent("WARN", "pair_failed");
      return;
    }

    char token[WIFI_SESSION_TOKEN_HEX_CHARS + 1] = {};
    generateToken(token);
    bool paired = false;
    if (!acquireDispatchLock()) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "internal_error"));
      return;
    }
    portENTER_CRITICAL(&_stateMux);
    if (!_session.active || _session.clientId == clientId) {
      _session = {};
      _session.active = true;
      _session.clientId = clientId;
      _session.generation = ++_nextSessionGeneration;
      strlcpy(_session.token, token, sizeof(_session.token));
      _session.leaseExpiryMs = now + WIFI_CONTROLLER_LEASE_MS;
      _session.lastAcceptedCommandMs = now;
      _pairingCode[0] = '\0';
      state->failedPairAttempts = 0;
      _globalPairWindowStartedMs = now;
      _globalFailedPairAttempts = 0;
      _pairingLockedUntilMs = 0;
      paired = true;
    }
    portEXIT_CRITICAL(&_stateMux);
    if (paired) {
      clearQueuedCommands();
    }
    releaseDispatchLock();

    if (!paired) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "controller_busy"));
      return;
    }

    _status->clearPairingCode();
    _status->setWifiStatus(true, _apSsid, _apIp, _ws->count(), true, false);
    logEvent("INFO", "paired");

    JsonDocument ackDoc;
    ackDoc["type"] = "paired";
    if (msg.hasRequestId) ackDoc["id"] = msg.requestId;
    ackDoc["ok"] = true;
    ackDoc["token"] = token;
    ackDoc["leaseMs"] = WIFI_CONTROLLER_LEASE_MS;
    String ackStr;
    serializeJson(ackDoc, ackStr);
    _ws->text(clientId, ackStr);

    broadcastTelemetry();
    sendEvents(clientId);
    return;
  }

  if (msg.type != WebMessageType::STOP) {
    bool controllerOwned = false;
    uint32_t sessionGeneration = 0;
    portENTER_CRITICAL(&_stateMux);
    controllerOwned = _session.active && _session.clientId == clientId;
    portEXIT_CRITICAL(&_stateMux);
    if (!controllerOwned) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "not_controller"));
      return;
    }
    if (!controllerMatches(clientId, msg, &sessionGeneration)) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "bad_token"));
      return;
    }
    if (!acceptRequestId(msg.requestId, sessionGeneration)) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "replayed_command"));
      return;
    }

    if (msg.command.intentId[0] != '\0') {
      bool duplicate = false;
      portENTER_CRITICAL(&_stateMux);
      if (_session.active && _session.clientId == clientId) {
        for (size_t i = 0; i < WifiControllerSession::MAX_RECENT_INTENTS; ++i) {
          if (_session.recentIntents[i][0] != '\0' && strcmp(_session.recentIntents[i], msg.command.intentId) == 0) {
            duplicate = true;
            break;
          }
        }
        if (!duplicate) {
           strlcpy(_session.recentIntents[_session.recentIntentsHead], msg.command.intentId, sizeof(_session.recentIntents[0]));
           _session.recentIntentsHead = (_session.recentIntentsHead + 1) % WifiControllerSession::MAX_RECENT_INTENTS;
        }
      }
      portEXIT_CRITICAL(&_stateMux);
      if (duplicate) {
        _ws->text(clientId, _protocol.generateExecResult(
            msg.command.intentId, true, "already_executed",
            _session.token,
            SafetySupervisor::stateName(_robot->safetyState()),
            SafetySupervisor::faultName(_robot->safetyFault())
        ));
        if (msg.hasRequestId) {
          _ws->text(clientId, _protocol.generateAck(msg.requestId, true, "ok", _router->currentEpoch()));
        }
        return;
      }
    }

    QueuedCommand qc;
    qc.command = msg.command;
    qc.enqueuedMs = now;
    qc.observedEpoch = _router->currentEpoch();
    qc.requestId = msg.hasRequestId ? msg.requestId : 0;
    qc.clientId = clientId;
    qc.sessionGeneration = sessionGeneration;

    if (enqueueCommand(qc)) {
      refreshControllerLease(clientId, sessionGeneration, now);
    } else {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "queue_full"));
    }
  } else {
    bool controllerOwned = false;
    bool tokenMatches = !msg.hasToken;
    if (!acquireDispatchLock()) {
      _ws->text(clientId, _protocol.generateError(msg.requestId, "internal_error"));
      return;
    }
    portENTER_CRITICAL(&_stateMux);
    controllerOwned = _session.active && _session.clientId == clientId;
    if (controllerOwned && msg.hasToken) {
      tokenMatches = constantTimeEquals(msg.token, _session.token, WIFI_SESSION_TOKEN_HEX_CHARS);
    }
    if (controllerOwned) {
      _session.leaseExpiryMs = now + WIFI_CONTROLLER_LEASE_MS;
      _session.lastAcceptedCommandMs = now;
    }
    portEXIT_CRITICAL(&_stateMux);
    if (!controllerOwned) {
      releaseDispatchLock();
      _ws->text(clientId, _protocol.generateError(msg.requestId, "not_controller"));
      return;
    }
    if (!tokenMatches) {
      releaseDispatchLock();
      _ws->text(clientId, _protocol.generateError(msg.requestId, "bad_token"));
      return;
    }
    requestEmergencyStopFromWifi(clientId, SafetyFault::NONE);
    releaseDispatchLock();
    if (msg.hasRequestId) {
      _ws->text(clientId, _protocol.generateAck(msg.requestId, true, "ok", _router->currentEpoch()));
    }
    return;
  }
}

void WifiControl::update() {
  if (!_running) return;
  
  uint32_t now = millis();

  bool pairingExpired = false;
  bool sessionActive = false;
  portENTER_CRITICAL(&_stateMux);
  if (_pairingCode[0] != '\0' && now - _pairingStartTime > WIFI_PAIRING_CODE_LIFETIME_MS) {
    _pairingCode[0] = '\0';
    pairingExpired = true;
  }
  sessionActive = _session.active;
  portEXIT_CRITICAL(&_stateMux);
  if (pairingExpired) {
    _status->clearPairingCode();
    _status->setWifiStatus(true, _apSsid, _apIp, _ws ? _ws->count() : 0, sessionActive, false);
    broadcastTelemetry();
  }

  if (_ws) _ws->cleanupClients();
  evictIdleUnpairedClients(now);
  syncStatus();

  bool leaseExpired = false;
  bool driveWatchdogExpired = false;
  uint32_t controllerId = 0;
  portENTER_CRITICAL(&_stateMux);
  if (_session.active) {
    controllerId = _session.clientId;
    if (static_cast<int32_t>(now - _session.leaseExpiryMs) >= 0) {
      leaseExpired = true;
    } else if (!_session.driveWatchdogStopped &&
               _session.lastActiveDriveMs != 0 &&
               now - _session.lastActiveDriveMs > WIFI_DRIVE_WATCHDOG_MS) {
      driveWatchdogExpired = true;
      _session.driveWatchdogStopped = true;
      _session.lastActiveDriveMs = 0;
    }
  }
  portEXIT_CRITICAL(&_stateMux);

  if (leaseExpired) {
    logEvent("WARN", "lease_expired");
    revokeController(SafetyFault::CONTROLLER_LEASE_EXPIRED);
  } else if (driveWatchdogExpired) {
    requestEmergencyStopFromWifi(controllerId, SafetyFault::DRIVE_WATCHDOG);
    logEvent("WARN", "drive_watchdog");
  }

  bool doPendingStop = false;
  SafetyFault stopFault = SafetyFault::NONE;
  portENTER_CRITICAL(&_queueMux);
  if (_pendingStop) {
    doPendingStop = true;
    stopFault = _pendingStopFault;
    _pendingStop = false;
  }
  portEXIT_CRITICAL(&_queueMux);

  if (doPendingStop) {
    if (stopFault == SafetyFault::NONE) {
      RobotCommand stopCmd;
      stopCmd.kind = CommandKind::STOP;
      stopCmd.source = ControlSource::WIFI;
      _router->execute(stopCmd);
    } else {
      _router->emergencyStop(stopFault);
    }
    portENTER_CRITICAL(&_stateMux);
    _session.driveWatchdogStopped = true;
    _session.lastActiveDriveMs = 0;
    portEXIT_CRITICAL(&_stateMux);
    broadcastTelemetry();
  }

  QueuedCommand qc;
  while (dequeueCommand(qc)) {
    bool sessionCurrent = false;
    bool superseded = false;
    const bool dispatchLocked = acquireDispatchLock();
    if (dispatchLocked) {
      sessionCurrent = commandSessionCurrent(qc);
    }
    if (sessionCurrent && _router->lastInterventionEpoch() > qc.observedEpoch) {
      if (qc.command.kind != CommandKind::STOP && qc.command.kind != CommandKind::DISARM) {
        superseded = true;
      }
    }

    bool executeIt = dispatchLocked && sessionCurrent && !superseded;
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
            const SafetyFault fault = _robot->safetyFault();
            const ObstacleSafetyStatus& obstacle = _robot->obstacleSafetyStatus();
            if (qc.command.driveMode == DriveMode::FORWARD &&
                obstacle.forwardMotionBlocked) {
              replyMsg = obstacle.rangeValid ? "obstacle_blocked" : "sensor_unavailable";
            } else if (fault == SafetyFault::OBSTACLE_BLOCKED) {
              replyMsg = "obstacle_blocked";
            } else if (fault == SafetyFault::RANGE_SENSOR_INVALID) {
              replyMsg = "sensor_unavailable";
            } else if (fault != SafetyFault::NONE) {
              replyMsg = SafetySupervisor::faultName(fault);
            }
          }
        }
        if (qc.command.kind == CommandKind::MOVE && ok) {
          portENTER_CRITICAL(&_stateMux);
          if (_session.active && _session.clientId == qc.clientId) {
            _session.lastActiveDriveMs = millis();
            _session.driveWatchdogStopped = false;
          }
          portEXIT_CRITICAL(&_stateMux);
        }
      }
    } else {
      if (!dispatchLocked) {
        replyMsg = "internal_error";
      } else if (!sessionCurrent) {
        replyMsg = "session_expired";
      } else {
        replyMsg = "superseded";
      }
    }
    if (dispatchLocked) {
      releaseDispatchLock();
    }

    if (qc.requestId > 0 && qc.clientId > 0) {
      _ws->text(qc.clientId, _protocol.generateAck(qc.requestId, ok, replyMsg, _router->currentEpoch()));
    }
    
    if (qc.command.intentId[0] != '\0') {
      _ws->text(qc.clientId, _protocol.generateExecResult(
          qc.command.intentId, ok, replyMsg,
          _session.token,
          SafetySupervisor::stateName(_robot->safetyState()),
          SafetySupervisor::faultName(_robot->safetyFault())
      ));
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
  t.protocolVersion = CONTROL_PROTOCOL_VERSION;
  t.configSchemaVersion = CONFIG_SCHEMA_VERSION;
  t.hardwareManifestVersion = HARDWARE_MANIFEST_VERSION;
  t.safetyPolicyVersion = SAFETY_POLICY_VERSION;
  t.wifiApRunning = _running && WIFI_START_SOFT_AP;
  t.wifiClientCount = _ws ? _ws->count() : 0;
  portENTER_CRITICAL(&_stateMux);
  t.controllerPresent = _session.active;
  t.pairingAvailable = _pairingCode[0] != '\0';
  portEXIT_CRITICAL(&_stateMux);
  
  if (_robot) {
    ActuatorCapabilities caps = _robot->actuatorCapabilities();
    t.hasDrive = caps.driveAvailable;
    t.fourWheelDrive = caps.fourWheelDrive;
    t.hasManipulators = caps.manipulatorAvailable;
    
    t.motorsArmed = _robot->isArmed();
    t.motorAllowedByFirmware = ALLOW_MOTOR_ARMING;
    t.safetyState = SafetySupervisor::stateName(_robot->safetyState());
    t.safetyFault = SafetySupervisor::faultName(_robot->safetyFault());
    t.safetyStateChangedMs = _robot->safetyStateChangedAtMs();
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
    case SafetyStopReason::PHYSICAL_ESTOP: t.lastSafetyStopReason = "physical_estop"; break;
    case SafetyStopReason::IMU_UNAVAILABLE: t.lastSafetyStopReason = "imu_unavailable"; break;
    case SafetyStopReason::IMU_INVALID: t.lastSafetyStopReason = "imu_invalid"; break;
    case SafetyStopReason::DRIVE_UNAVAILABLE: t.lastSafetyStopReason = "drive_unavailable"; break;
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

  const ImuReading& imu = _robot->imuReading();
  t.imuAvailable = imu.available;
  t.imuValid = imu.valid;
  if (imu.valid) {
    t.imuSampleTimeMs = imu.sampleTimeMs;
    t.accelXG = imu.accelXG;
    t.accelYG = imu.accelYG;
    t.accelZG = imu.accelZG;
    t.gyroXDps = imu.gyroXDps;
    t.gyroYDps = imu.gyroYDps;
    t.gyroZDps = imu.gyroZDps;
  }

  String telemetryJson = _protocol.generateTelemetry(t);
  if (_ws) {
    _ws->textAll(telemetryJson);
    if (_robot) {
      _ws->textAll(_protocol.generateRuntimeSnapshot(0, _robot->runtimeSnapshot()));
    }
  }
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
