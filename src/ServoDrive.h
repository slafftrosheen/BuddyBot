#pragma once
#include "IDriveBase.h"
#include "Servo8Bus.h"

struct ServoDriveLayout {
  bool isFourWheel;
  ServoRole frontLeft;
  ServoRole rearLeft;
  ServoRole frontRight;
  ServoRole rearRight;
};

class ServoDrive : public IDriveBase {
public:
  ServoDrive(Servo8Bus* bus, const ServoDriveLayout& layout);

  bool begin() override;
  void update() override;
  bool isConnected() const override;
  bool isArmed() const override;
  void arm() override;
  void disarm() override;
  void emergencyStop() override;
  void drive(const DriveCommand& cmd) override;
  DriveMode driveMode() const override;

  bool isFourWheel() const;

private:
  Servo8Bus* _bus = nullptr;
  ServoDriveLayout _layout;
  bool _armed = false;
  DriveMode _mode = DriveMode::STOPPED;
  uint32_t _stopAtMs = 0;

  void setLeftDrive(bool forward);
  void setRightDrive(bool forward);
  void stopDrive();
  
  void applyDrivePulse(ServoRole role, bool forward);
};
