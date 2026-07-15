#pragma once
#include "HalTypes.h"

class IDriveBase {
public:
  virtual ~IDriveBase() = default;
  virtual bool begin() = 0;
  virtual void update() = 0;
  virtual bool isConnected() const = 0;
  virtual bool isArmed() const = 0;
  virtual void arm() = 0;
  virtual void disarm() = 0;
  virtual void emergencyStop() = 0;
  virtual void drive(const DriveCommand& cmd) = 0;
  virtual DriveMode driveMode() const = 0;
};
