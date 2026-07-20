#pragma once
#include "RobotAPI.h"
#include "RobotHal.h"

class SystemStatus {
public:
  void begin(RobotAPI* robot, RobotHal* hal);
  void printStatus() const;

  void setPairingCode(const char* code);
  void clearPairingCode();
  const char* getPairingCode() const { return _pairingCode; }

  void setWifiStatus(const char* ssid, const char* ip, uint8_t clients, bool hasController);
  const char* getWifiSsid() const { return _wifiSsid; }
  const char* getWifiIp() const { return _wifiIp; }
  uint8_t getWifiClients() const { return _wifiClients; }
  bool getWifiHasController() const { return _wifiHasController; }

private:
  RobotAPI* _robot = nullptr;
  RobotHal* _hal = nullptr;

  char _pairingCode[16] = {0};
  char _wifiSsid[32] = {0};
  char _wifiIp[16] = {0};
  uint8_t _wifiClients = 0;
  bool _wifiHasController = false;
};
