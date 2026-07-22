#pragma once
#include "RobotAPI.h"
#include "RobotHal.h"
#include "BootDiagnostics.h"
#include "EventLog.h"

struct FirmwareIdentity {
  const char* name;
  const char* version;
  const char* channel;
  const char* buildProfile;
  const char* compileDate;
  const char* compileTime;
};

FirmwareIdentity getFirmwareIdentity();

class SystemStatus {
public:
  void begin(RobotAPI* robot, RobotHal* hal, BootDiagnostics* diag = nullptr);
  void printStatus() const;
  void printEvents() const;

  void setPairingCode(const char* code);
  void clearPairingCode();
  const char* getPairingCode() const { return _pairingCode; }

  void setWifiStatus(bool running, const char* ssid, const char* ip, uint8_t clients, bool hasController, bool pairingAvailable);
  
  bool wifiRunning() const { return _wifiRunning; }
  bool wifiHasController() const { return _wifiHasController; }
  bool wifiPairingAvailable() const { return _wifiPairingAvailable; }
  uint8_t wifiClients() const { return _wifiClients; }
  const char* wifiSsid() const { return _wifiSsid; }
  const char* wifiIp() const { return _wifiIp; }

private:
  RobotAPI* _robot = nullptr;
  RobotHal* _hal = nullptr;
  BootDiagnostics* _diag = nullptr;

  char _pairingCode[16] = {0};
  bool _wifiRunning = false;
  char _wifiSsid[32] = {0};
  char _wifiIp[16] = {0};
  uint8_t _wifiClients = 0;
  bool _wifiHasController = false;
  bool _wifiPairingAvailable = false;
};
