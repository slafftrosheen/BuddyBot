#pragma once
#include <memory>
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
  IJoint* accessory(uint8_t index) const;
  const RobotBuildConfig& buildConfig() const { return _config; }

private:
  RobotBuildConfig _config;

  Servo8Bus _servoBus;
  SonicRangeSensor _sonic;

  std::unique_ptr<ServoDrive> _servoDrive;
  std::unique_ptr<ServoJoint> _headServo;
  std::unique_ptr<ServoJoint> _leftArmServo;
  std::unique_ptr<ServoJoint> _rightArmServo;
  std::unique_ptr<ServoJoint> _acc1Servo;
  std::unique_ptr<ServoJoint> _acc2Servo;
  std::unique_ptr<ServoJoint> _acc3Servo;

  std::unique_ptr<RollerBus> _rollerBus1;
  std::unique_ptr<RollerBus> _rollerBus2;
  std::unique_ptr<RollerDrive> _rollerDrive;
  std::unique_ptr<RollerJoint> _rollerJoint1;
  std::unique_ptr<RollerJoint> _rollerJoint2;

  IDriveBase* _drive = nullptr;
  IJoint* _head = nullptr;
  IJoint* _leftArm = nullptr;
  IJoint* _rightArm = nullptr;
  IJoint* _acc1 = nullptr;
  IJoint* _acc2 = nullptr;
  IJoint* _acc3 = nullptr;
  IRangeSensor* _range = nullptr;

  void release();
};
