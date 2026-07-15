#pragma once
#include "IDriveBase.h"
#include "Servo8Bus.h"

class ServoDrive : public IDriveBase {
public:
  explicit ServoDrive(Servo8Bus* bus);

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
  Servo8Bus* _bus = nullptr;
  bool _armed = false;
  DriveMode _mode = DriveMode::STOPPED;
  uint32_t _stopAtMs = 0;

  void wheels(uint16_t leftUs, uint16_t rightUs);
};
