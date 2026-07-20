#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"
#include "ControlRouter.h"
#include "RobotAPI.h"
#include "SystemStatus.h"
#include "WebControlProtocol.h"

struct QueuedCommand {
  RobotCommand cmd;
  uint32_t enqueuedMs;
  uint32_t enqueuedEpoch;
  uint32_t msgId;
  uint32_t clientId;
};

class WifiControl {
public:
  void begin(ControlRouter* router, RobotAPI* robot, SystemStatus* status);
  void update();

private:
  void startSoftAP();
  void handleWebSocketMessage(void* arg, uint8_t* data, size_t len, uint32_t clientId);
  void sendStop();
  void broadcastTelemetry();
  void logEvent(const char* severity, const char* code);
  void generatePairingCode();
  void revokeController();
  String generateToken();
  void sendEvents(uint32_t clientId);

  ControlRouter* _router = nullptr;
  RobotAPI* _robot = nullptr;
  SystemStatus* _status = nullptr;
  WebControlProtocol _protocol;

  AsyncWebServer* _server = nullptr;
  AsyncWebSocket* _ws = nullptr;

  uint32_t _controllerId = 0;
  String _sessionToken = "";
  uint32_t _lastDriveCommandMs = 0;
  uint32_t _leaseExpiryMs = 0;
  
  uint32_t _lastTelemetryMs = 0;
  
  char _pairingCode[16] = {0};
  uint32_t _pairingStartTime = 0;

  // Rate limiting
  uint32_t _lastSecondMs = 0;
  uint8_t _cmdCountThisSecond = 0;
  uint8_t _pairAttemptsThisMinute = 0;
  uint32_t _lastMinuteMs = 0;

  // Event Log
  static const size_t EVENT_LOG_SIZE = 10;
  EventLogEntry _eventLog[EVENT_LOG_SIZE];
  size_t _eventLogHead = 0;

  // Queue
  static const size_t CMD_QUEUE_SIZE = 16;
  QueuedCommand _cmdQueue[CMD_QUEUE_SIZE];
  size_t _cmdQueueHead = 0;
  size_t _cmdQueueTail = 0;
  portMUX_TYPE _queueMux = portMUX_INITIALIZER_UNLOCKED;
};
