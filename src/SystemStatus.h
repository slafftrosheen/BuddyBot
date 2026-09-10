#pragma once
#include "RobotAPI.h"
#include "RobotHal.h"
#include "BootDiagnostics.h"
#include "EventLog.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#endif

class SystemHealth;

struct FirmwareIdentity {
  const char* name;
  const char* version;
  const char* channel;
  const char* buildProfile;
  const char* compileDate;
  const char* compileTime;
};

FirmwareIdentity getFirmwareIdentity();

struct WifiStatusSnapshot {
  bool running = false;
  char ssid[32] = {0};
  char ip[16] = {0};
  uint8_t clients = 0;
  bool hasController = false;
  bool pairingAvailable = false;
  char pairingCode[16] = {0};
};

class SystemStatus {
public:
  void begin(
    RobotAPI* robot,
    RobotHal* hal,
    BootDiagnostics* diag = nullptr,
    SystemHealth* health = nullptr
  );
  void printStatus() const;
  void printEvents() const;

  void setPairingCode(const char* code);
  void clearPairingCode();
  const char* getPairingCode() const { return _pairingCode; }
  void getPairingCode(char* out, size_t maxLen) const;

  void setWifiStatus(bool running, const char* ssid, const char* ip, uint8_t clients, bool hasController, bool pairingAvailable);
  void getWifiSnapshot(WifiStatusSnapshot& out) const;
  
  bool wifiRunning() const;
  bool wifiHasController() const;
  bool wifiPairingAvailable() const;
  uint8_t wifiClients() const;
  const char* wifiSsid() const;
  const char* wifiIp() const;

private:
  void lock() const {
#if defined(ARDUINO_ARCH_ESP32)
    portENTER_CRITICAL(&_statusMux);
#endif
  }

  void unlock() const {
#if defined(ARDUINO_ARCH_ESP32)
    portEXIT_CRITICAL(&_statusMux);
#endif
  }

  RobotAPI* _robot = nullptr;
  RobotHal* _hal = nullptr;
  BootDiagnostics* _diag = nullptr;
  SystemHealth* _health = nullptr;

#if defined(ARDUINO_ARCH_ESP32)
  mutable portMUX_TYPE _statusMux = portMUX_INITIALIZER_UNLOCKED;
#endif

  char _pairingCode[16] = {0};
  bool _wifiRunning = false;
  char _wifiSsid[32] = {0};
  char _wifiIp[16] = {0};
  uint8_t _wifiClients = 0;
  bool _wifiHasController = false;
  bool _wifiPairingAvailable = false;
};
