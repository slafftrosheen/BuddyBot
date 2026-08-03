#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"
#include "ControlRouter.h"
#include "RobotAPI.h"
#include "SystemStatus.h"
#include "EventLog.h"
#include "WebControlProtocol.h"

struct QueuedCommand {
  RobotCommand command {};
  uint32_t enqueuedMs = 0;
  uint32_t observedEpoch = 0;
  uint32_t requestId = 0;
  uint32_t clientId = 0;
};

struct WifiClientState {
  uint32_t clientId = 0;
  bool connected = false;

  uint32_t commandWindowStartedMs = 0;
  uint8_t commandsInWindow = 0;

  uint32_t pairWindowStartedMs = 0;
  uint8_t failedPairAttempts = 0;
};

struct WifiControllerSession {
  uint32_t clientId = 0;
  bool active = false;

  char token[WIFI_SESSION_TOKEN_HEX_CHARS + 1] = {};

  uint32_t leaseExpiryMs = 0;
  uint32_t lastAcceptedCommandMs = 0;
  uint32_t lastActiveDriveMs = 0;
  bool driveWatchdogStopped = false;
};

class WifiControl {
public:
  void begin(ControlRouter* router, RobotAPI* robot, SystemStatus* status);
  void update();
  
  bool start();
  void stop();
  bool running() const;
  bool controllerPresent() const;
  bool pairingAvailable() const;
  uint8_t clientCount() const;
  bool requestNewPairingCode();
  const char* apSsid() const;
  const char* apIp() const;

private:
  bool startSoftAP();
  void syncStatus();
  void handleWebSocketMessage(void* arg, uint8_t* data, size_t len, uint32_t clientId);
  void broadcastTelemetry();
  void broadcastEvents();
  void logEvent(const char* severity, const char* code);
  void generatePairingCode();
  void revokeController();
  void generateToken(char* outBuffer);
  void sendEvents(uint32_t clientId);
  
  bool enqueueCommand(const QueuedCommand& cmd);
  bool dequeueCommand(QueuedCommand& cmd);
  void requestEmergencyStopFromWifi(uint32_t clientId);
  WifiClientState* getClientState(uint32_t clientId);
  bool constantTimeEquals(const char* a, const char* b, size_t n);

  ControlRouter* _router = nullptr;
  RobotAPI* _robot = nullptr;
  SystemStatus* _status = nullptr;
  WebControlProtocol _protocol;

  AsyncWebServer* _server = nullptr;
  AsyncWebSocket* _ws = nullptr;
  bool _running = false;
  
  char _apSsid[32] = {0};
  char _apIp[16] = {0};

  WifiControllerSession _session;
  WifiClientState _clients[WIFI_MAX_CLIENTS];

  uint32_t _lastTelemetryMs = 0;
  
  char _pairingCode[WIFI_PAIRING_DIGITS + 1] = {0};
  uint32_t _pairingStartTime = 0;

  // Queue
  QueuedCommand _cmdQueue[WIFI_COMMAND_QUEUE_CAPACITY];
  size_t _cmdQueueHead = 0;
  size_t _cmdQueueTail = 0;
  size_t _cmdQueueSize = 0;
  portMUX_TYPE _queueMux = portMUX_INITIALIZER_UNLOCKED;

  volatile bool _pendingStop = false;
  volatile uint32_t _pendingStopClientId = 0;
};
