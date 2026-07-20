#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"
#include "ControlRouter.h"
#include "RobotAPI.h"
#include "WebControlProtocol.h"

class WifiControl {
public:
  void begin(ControlRouter* router, RobotAPI* robot);
  void update();

private:
  void startSoftAP();
  void handleWebSocketMessage(void* arg, uint8_t* data, size_t len, uint32_t clientId);
  void sendStop();
  void broadcastTelemetry();

  ControlRouter* _router = nullptr;
  RobotAPI* _robot = nullptr;
  WebControlProtocol _protocol;

  AsyncWebServer* _server = nullptr;
  AsyncWebSocket* _ws = nullptr;

  uint32_t _controllerId = 0;
  uint32_t _lastDriveCommandMs = 0;
  uint32_t _lastTelemetryMs = 0;
};
