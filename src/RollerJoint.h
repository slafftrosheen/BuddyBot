#pragma once
#include "IJoint.h"
#include "HalTypes.h"
#include "RollerBus.h"

class RollerJoint : public IJoint {
public:
  RollerJoint(RollerBus* bus, const JointLimits& limits);

  bool begin() override;
  bool isConnected() const override;
  void moveTo(int16_t value, uint16_t durationMs = 0) override;
  int16_t current() const override;
  void rest() override;

private:
  RollerBus* _bus = nullptr;
  JointLimits _limits;
  int16_t _current = 0;
};
