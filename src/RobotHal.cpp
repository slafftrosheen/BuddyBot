#include "RobotHal.h"

IJoint* RobotHal::accessory(uint8_t index) const {
  if (index == 1) return _acc1;
  if (index == 2) return _acc2;
  if (index == 3) return _acc3;
  return nullptr;
}

void RobotHal::release() {
  _drive = nullptr;
  _head = nullptr;
  _leftArm = nullptr;
  _rightArm = nullptr;
  _acc1 = nullptr;
  _acc2 = nullptr;
  _acc3 = nullptr;
  _range = nullptr;

  _servoDrive.reset();
  _headServo.reset();
  _leftArmServo.reset();
  _rightArmServo.reset();
  _acc1Servo.reset();
  _acc2Servo.reset();
  _acc3Servo.reset();

  // Correct cleanup order: destroy drives and joints before buses
  _rollerDrive.reset();
  _rollerJoint1.reset();
  _rollerJoint2.reset();
  
  _rollerBus1.reset();
  _rollerBus2.reset();
}

bool RobotHal::begin(const RobotBuildConfig& config) {
  release();
  _config = config;

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);
  _servoBus.begin();

  if (config.driveType == DriveControllerType::SERVO8_CONTINUOUS) {
    _servoDrive = std::unique_ptr<ServoDrive>(new ServoDrive(&_servoBus));
    if (_servoDrive) {
      if (!_servoDrive->begin()) return false;
      _drive = _servoDrive.get();
    }
  } else if (config.driveType == DriveControllerType::ROLLER_UNIT) {
    if (!config.useRoller1ForDrive || !config.useRoller2ForDrive) {
      Serial.println("ERR: Dual Roller drive requires both rollers configured");
      return false;
    }
    
    _rollerBus1 = std::unique_ptr<RollerBus>(new RollerBus(Wire, ROLLER1_ADDR));
    _rollerBus2 = std::unique_ptr<RollerBus>(new RollerBus(Wire, ROLLER2_ADDR));
    _rollerDrive = std::unique_ptr<RollerDrive>(new RollerDrive(_rollerBus1.get(), _rollerBus2.get()));

    if (!_rollerDrive->begin()) {
      Serial.println("ERR: Roller drive init failed");
      return false;
    }
    _drive = _rollerDrive.get();
  }

  if (config.headType == JointControllerType::SERVO8_POSITION) {
    _headServo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, HEAD_CHANNEL, JointLimits{0, 180, HEAD_CENTER_DEG}));
    if (_headServo) _headServo->begin();
    _head = _headServo.get();
  }

  if (config.leftArmType == JointControllerType::SERVO8_POSITION) {
    _leftArmServo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, LEFT_ARM_CHANNEL, JointLimits{0, 180, ARM_LEFT_REST_DEG}));
    if (_leftArmServo) _leftArmServo->begin();
    _leftArm = _leftArmServo.get();
  }

  if (config.rightArmType == JointControllerType::SERVO8_POSITION) {
    _rightArmServo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, RIGHT_ARM_CHANNEL, JointLimits{0, 180, ARM_RIGHT_REST_DEG}));
    if (_rightArmServo) _rightArmServo->begin();
    _rightArm = _rightArmServo.get();
  }

  if (config.accessory1Type == AccessoryControllerType::SERVO8_POSITION) {
    _acc1Servo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, ACCESSORY_CHANNEL_1, JointLimits{0, 180, ACCESSORY_REST_DEG}));
    if (_acc1Servo) _acc1Servo->begin();
    _acc1 = _acc1Servo.get();
  }
  
  if (config.accessory2Type == AccessoryControllerType::SERVO8_POSITION) {
    _acc2Servo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, ACCESSORY_CHANNEL_2, JointLimits{0, 180, ACCESSORY_REST_DEG}));
    if (_acc2Servo) _acc2Servo->begin();
    _acc2 = _acc2Servo.get();
  }
  
  if (config.accessory3Type == AccessoryControllerType::SERVO8_POSITION) {
    _acc3Servo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, ACCESSORY_CHANNEL_3, JointLimits{0, 180, ACCESSORY_REST_DEG}));
    if (_acc3Servo) _acc3Servo->begin();
    _acc3 = _acc3Servo.get();
  }

  if (config.rangeSensorType == RangeSensorType::SONIC_I2C) {
    if (_sonic.begin()) {
      _range = &_sonic;
    } else {
      Serial.println("WARN: Range sensor failed to init");
    }
  }

  return true;
}

void RobotHal::update() {
  if (_drive) _drive->update();
  if (_range) _range->update();
}
