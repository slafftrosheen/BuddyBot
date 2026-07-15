#pragma once
#include "IJoint.h"
#include "HalTypes.h"
#include "Servo8Bus.h"

class ServoJoint : public IJoint {
public:
  ServoJoint(Servo8Bus* bus, uint8_t channel, const JointLimits& limits);

  bool begin() override;
  bool isConnected() const override;
  void moveTo(int16_t value) override;
  int16_t current() const override;
  void rest() override;

private:
  Servo8Bus* _bus = nullptr;
  uint8_t _channel = 0;
  JointLimits _limits;
  int16_t _current = 90;
};
