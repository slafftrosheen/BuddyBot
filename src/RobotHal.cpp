#include "RobotHal.h"

void RobotHal::release() {
  _drive = nullptr;
  _head = nullptr;
  _leftArm = nullptr;
  _rightArm = nullptr;
  _range = nullptr;

  delete _servoDrive; _servoDrive = nullptr;
  delete _headServo; _headServo = nullptr;
  delete _leftArmServo; _leftArmServo = nullptr;
  delete _rightArmServo; _rightArmServo = nullptr;

  delete _rollerBus1; _rollerBus1 = nullptr;
  delete _rollerBus2; _rollerBus2 = nullptr;
  delete _rollerDrive; _rollerDrive = nullptr;
  delete _rollerJoint1; _rollerJoint1 = nullptr;
  delete _rollerJoint2; _rollerJoint2 = nullptr;
}

bool RobotHal::begin(const RobotBuildConfig& config) {
  release();
  _config = config;

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
  _servoBus.begin();

  if (config.driveType == DriveControllerType::SERVO8_CONTINUOUS) {
    _servoDrive = new ServoDrive(&_servoBus);
    if (_servoDrive) _servoDrive->begin();
    _drive = _servoDrive;
  } else if (config.driveType == DriveControllerType::ROLLER_UNIT) {
    RollerBus* b1 = new RollerBus(ROLLER1_ADDR);
    if (config.useRoller2ForDrive) {
      RollerBus* b2 = new RollerBus(ROLLER2_ADDR);
      _rollerBus2 = b2;
    }
    _rollerBus1 = b1;
    _rollerDrive = new RollerDrive(b1);
    if (_rollerDrive) _rollerDrive->begin();
    _drive = _rollerDrive;
  }

  if (config.headType == JointControllerType::SERVO8_POSITION) {
    _headServo = new ServoJoint(&_servoBus, HEAD_CHANNEL, JointLimits{0, 180, HEAD_CENTER_DEG});
    if (_headServo) _headServo->begin();
    _head = _headServo;
  }

  if (config.leftArmType == JointControllerType::SERVO8_POSITION) {
    _leftArmServo = new ServoJoint(&_servoBus, LEFT_ARM_CHANNEL, JointLimits{0, 180, ARM_LEFT_REST_DEG});
    if (_leftArmServo) _leftArmServo->begin();
    _leftArm = _leftArmServo;
  }

  if (config.rightArmType == JointControllerType::SERVO8_POSITION) {
    _rightArmServo = new ServoJoint(&_servoBus, RIGHT_ARM_CHANNEL, JointLimits{0, 180, ARM_RIGHT_REST_DEG});
    if (_rightArmServo) _rightArmServo->begin();
    _rightArm = _rightArmServo;
  }

  if (config.rangeSensorType == RangeSensorType::SONIC_I2C) {
    _sonic.begin();
    _range = &_sonic;
  }

  return true;
}

void RobotHal::update() {
  if (_drive) _drive->update();
  if (_range) _range->update();
}
