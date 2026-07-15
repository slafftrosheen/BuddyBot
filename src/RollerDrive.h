#pragma once
#include "IDriveBase.h"
#include "RollerBus.h"

class RollerDrive : public IDriveBase {
public:
  explicit RollerDrive(RollerBus* bus);

  bool begin() override;
  void update() override;
  bool isConnected() const override;
  bool isArmed() const override;
  void arm() override;
  void disarm() override;
  void emergencyStop() override;
  void drive(const DriveCommand& cmd) override;
  DriveMode driveMode() const override;

private:
  RollerBus* _bus = nullptr;
  bool _armed = false;
  DriveMode _mode = DriveMode::STOPPED;
  uint32_t _stopAtMs = 0;
};
