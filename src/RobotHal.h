#pragma once
#include "IDriveBase.h"
#include "IJoint.h"
#include "IRangeSensor.h"
#include "Servo8Bus.h"
#include "ServoDrive.h"
#include "ServoJoint.h"
#include "SonicRangeSensor.h"
#include "RollerBus.h"
#include "RollerDrive.h"
#include "RollerJoint.h"
#include "Config.h"

class RobotHal {
public:
  bool begin(const RobotBuildConfig& config);
  void update();

  IDriveBase* drive() const { return _drive; }
  IJoint* head() const { return _head; }
  IJoint* leftArm() const { return _leftArm; }
  IJoint* rightArm() const { return _rightArm; }
  IRangeSensor* range() const { return _range; }
  const RobotBuildConfig& buildConfig() const { return _config; }

private:
  RobotBuildConfig _config;

  Servo8Bus _servoBus;
  SonicRangeSensor _sonic;

  ServoDrive* _servoDrive = nullptr;
  ServoJoint* _headServo = nullptr;
  ServoJoint* _leftArmServo = nullptr;
  ServoJoint* _rightArmServo = nullptr;
  RollerBus* _rollerBus1 = nullptr;
  RollerBus* _rollerBus2 = nullptr;
  RollerDrive* _rollerDrive = nullptr;
  RollerJoint* _rollerJoint1 = nullptr;
  RollerJoint* _rollerJoint2 = nullptr;

  IDriveBase* _drive = nullptr;
  IJoint* _head = nullptr;
  IJoint* _leftArm = nullptr;
  IJoint* _rightArm = nullptr;
  IRangeSensor* _range = nullptr;

  void release();
};
