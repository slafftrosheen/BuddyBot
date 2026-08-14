#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <freertos/semphr.h>
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
  uint32_t sessionGeneration = 0;
};

struct WifiClientState {
  uint32_t clientId = 0;
  bool connected = false;
  uint32_t connectedSinceMs = 0;
  uint32_t lastActivityMs = 0;

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
  bool hasLastRequestId = false;
  uint32_t lastRequestId = 0;
  uint32_t generation = 0;

  struct RecentIntent {
    char intentId[37];
    char status[16];
  };
  static const size_t MAX_RECENT_INTENTS = 4;
  RecentIntent recentIntents[MAX_RECENT_INTENTS] = {};
  size_t recentIntentsHead = 0;
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
  void revokeController(SafetyFault fault = SafetyFault::CONTROLLER_DISCONNECTED);
  void generateToken(char* outBuffer);
  void sendEvents(uint32_t clientId);
  
  bool enqueueCommand(const QueuedCommand& cmd);
  bool dequeueCommand(QueuedCommand& cmd);
  void clearQueuedCommands();
  void requestEmergencyStopFromWifi(uint32_t clientId, SafetyFault fault);
  WifiClientState* getClientState(uint32_t clientId);
  bool constantTimeEquals(const char* a, const char* b, size_t n) const;
  bool controllerMatches(
    uint32_t clientId,
    const WebParsedMessage& message,
    uint32_t* sessionGeneration = nullptr
  ) const;
  bool acceptRequestId(uint32_t requestId, uint32_t sessionGeneration);
  bool commandSessionCurrent(const QueuedCommand& command) const;
  void refreshControllerLease(uint32_t clientId, uint32_t sessionGeneration, uint32_t nowMs);
  bool acquireDispatchLock();
  void releaseDispatchLock();
  void recordPairFailure(uint32_t nowMs);
  bool pairingRateLimited(uint32_t nowMs) const;
  void evictIdleUnpairedClients(uint32_t nowMs);

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
  mutable portMUX_TYPE _stateMux = portMUX_INITIALIZER_UNLOCKED;
  SemaphoreHandle_t _dispatchMutex = nullptr;

  volatile bool _pendingStop = false;
  volatile uint32_t _pendingStopClientId = 0;
  volatile SafetyFault _pendingStopFault = SafetyFault::DRIVE_WATCHDOG;

  uint32_t _globalPairWindowStartedMs = 0;
  uint8_t _globalFailedPairAttempts = 0;
  uint32_t _pairingLockedUntilMs = 0;
  uint32_t _nextSessionGeneration = 0;
};
