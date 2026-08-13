#include "RobotHal.h"
#include "BuildProfiles.h"

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

  Wire1.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY);

  const ServoChannelConfig* servoConfig = getActiveServoConfig();
  if (!validateBuildConfig(config, servoConfig)) {
    Serial.println("ERR: Unsupported or invalid build profile");
    return false;
  }
  _servoBus.configureAll(servoConfig);
  _servoBus.begin();

  if (config.driveType == DriveControllerType::SERVO8_CONTINUOUS) {
    ServoDriveLayout layout;
    layout.isFourWheel = _servoBus.hasRole(ServoRole::DRIVE_REAR_LEFT);
    layout.frontLeft = ServoRole::DRIVE_FRONT_LEFT;
    layout.frontRight = ServoRole::DRIVE_FRONT_RIGHT;
    layout.rearLeft = ServoRole::DRIVE_REAR_LEFT;
    layout.rearRight = ServoRole::DRIVE_REAR_RIGHT;
    
    _servoDrive = std::unique_ptr<ServoDrive>(new ServoDrive(&_servoBus, layout));
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
    if (_servoBus.hasRole(ServoRole::HEAD)) {
      _headServo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, ServoRole::HEAD));
      if (_headServo) _headServo->begin();
      _head = _headServo.get();
    }
  }

  if (config.leftArmType == JointControllerType::SERVO8_POSITION) {
    if (_servoBus.hasRole(ServoRole::LEFT_ARM)) {
      _leftArmServo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, ServoRole::LEFT_ARM));
      if (_leftArmServo) _leftArmServo->begin();
      _leftArm = _leftArmServo.get();
    }
  }

  if (config.rightArmType == JointControllerType::SERVO8_POSITION) {
    if (_servoBus.hasRole(ServoRole::RIGHT_ARM)) {
      _rightArmServo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, ServoRole::RIGHT_ARM));
      if (_rightArmServo) _rightArmServo->begin();
      _rightArm = _rightArmServo.get();
    }
  }

  if (config.accessory1Type == AccessoryControllerType::SERVO8_POSITION) {
    if (_servoBus.hasRole(ServoRole::ACCESSORY_1)) {
      _acc1Servo = std::unique_ptr<ServoJoint>(new ServoJoint(&_servoBus, ServoRole::ACCESSORY_1));
      if (_acc1Servo) _acc1Servo->begin();
      _acc1 = _acc1Servo.get();
    }
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
  uint32_t nowMs = millis();
  
  if (_drive) _drive->update();
  
  if (_headServo) _headServo->update(nowMs);
  if (_leftArmServo) _leftArmServo->update(nowMs);
  if (_rightArmServo) _rightArmServo->update(nowMs);
  if (_acc1Servo) _acc1Servo->update(nowMs);
  if (_acc2Servo) _acc2Servo->update(nowMs);
  if (_acc3Servo) _acc3Servo->update(nowMs);
  
  if (_range) _range->update();
}

ActuatorCapabilities RobotHal::capabilities() const {
  ActuatorCapabilities caps = {};
  caps.servoBusConnected = _servoBus.isConnected();
  caps.driveAvailable = _drive != nullptr && _drive->isConnected();
  
  if (_drive && _config.driveType == DriveControllerType::SERVO8_CONTINUOUS) {
    ServoDrive* sd = static_cast<ServoDrive*>(_drive); // Safe since we know type
    caps.fourWheelDrive = sd->isFourWheel();
    caps.driveServoCount = caps.fourWheelDrive ? 4 : 2;
  }
  
  caps.headAvailable = _head != nullptr;
  caps.leftArmAvailable = _leftArm != nullptr;
  caps.rightArmAvailable = _rightArm != nullptr;
  caps.accessory1Available = _acc1 != nullptr;
  
  caps.manipulatorCount = (caps.headAvailable ? 1 : 0) + 
                          (caps.leftArmAvailable ? 1 : 0) + 
                          (caps.rightArmAvailable ? 1 : 0) + 
                          (caps.accessory1Available ? 1 : 0);
  caps.manipulatorAvailable = caps.manipulatorCount > 0;
  
  return caps;
}

ManipulatorState RobotHal::manipulatorState(ManipulatorId id) const {
  ManipulatorState state = {};
  IJoint* joint = nullptr;
  switch (id) {
    case ManipulatorId::HEAD: joint = _head; break;
    case ManipulatorId::LEFT_ARM: joint = _leftArm; break;
    case ManipulatorId::RIGHT_ARM: joint = _rightArm; break;
    case ManipulatorId::ACCESSORY_1: joint = _acc1; break;
    default: break;
  }
  
  if (joint) {
    state.available = true;
    state.currentDeg = joint->current();
    ServoJoint* sj = static_cast<ServoJoint*>(joint);
    if (sj) {
      state.targetDeg = sj->target();
      state.moving = sj->motionActive();
      state.motionState = sj->motionState();
    } else {
      state.targetDeg = state.currentDeg;
      state.moving = false;
      state.motionState = JointMotionState::IDLE;
    }
  } else {
    state.available = false;
    state.motionState = JointMotionState::UNAVAILABLE;
  }
  
  return state;
}

void RobotHal::writeRawPulse(ServoRole role, uint16_t pulseUs) {
  if (_servoBus.isConnected()) {
    const ServoChannelConfig* cfg = _servoBus.configForRole(role);
    if (cfg && cfg->enabled) {
      _servoBus.writePulse(cfg->channel, pulseUs);
    }
  }
}
