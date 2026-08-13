#pragma once
#include "Config.h"
#include "ServoConfig.h"

struct NamedBuildProfile {
  BuildProfileId id;
  const char* name;
  RobotBuildConfig config;
};

struct HardwareManifest {
  uint16_t schemaVersion;
  BuildProfileId profileId;
  const char* profileName;
  RobotBuildConfig config;
};

extern const RobotBuildConfig CUSTOM_BUILD;
extern const NamedBuildProfile BUILD_PROFILES[];
extern const size_t BUILD_PROFILE_COUNT;

RobotBuildConfig getActiveBuildConfig();
const char* getActiveBuildName();
HardwareManifest getActiveHardwareManifest();
const ServoChannelConfig* getActiveServoConfig();
const NamedBuildProfile* findBuildProfile(BuildProfileId id);
void printBuildProfile(const RobotBuildConfig& config);
void printAllBuildProfiles();
bool validateBuildConfig(const RobotBuildConfig& config, const ServoChannelConfig* servos);
