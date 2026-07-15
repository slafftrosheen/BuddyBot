#pragma once
#include "Config.h"

struct NamedBuildProfile {
  BuildProfileId id;
  const char* name;
  RobotBuildConfig config;
};

extern const RobotBuildConfig CUSTOM_BUILD;
extern const NamedBuildProfile BUILD_PROFILES[];
extern const size_t BUILD_PROFILE_COUNT;

RobotBuildConfig getActiveBuildConfig();
const char* getActiveBuildName();
const NamedBuildProfile* findBuildProfile(BuildProfileId id);
void printBuildProfile(const RobotBuildConfig& config);
void printAllBuildProfiles();
