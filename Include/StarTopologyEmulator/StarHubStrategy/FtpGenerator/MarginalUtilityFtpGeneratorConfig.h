#pragma once

#include <variant>

#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F1LinearUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F2LogarithmicUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F3AlphaFairUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F4SigmoidalUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F5HardDeadlineUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F6CostOfDelayUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F7QuadraticBacklogUtilityConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/F8CesUtilityConfig.h"

namespace starTopologyEmulator
{

using MarginalUtilityFtpGeneratorConfig = std::variant<
    F1LinearUtilityConfig,
    F2LogarithmicUtilityConfig,
    F3AlphaFairUtilityConfig,
    F4SigmoidalUtilityConfig,
    F5HardDeadlineUtilityConfig,
    F6CostOfDelayUtilityConfig,
    F7QuadraticBacklogUtilityConfig,
    F8CesUtilityConfig>;

} // namespace starTopologyEmulator
