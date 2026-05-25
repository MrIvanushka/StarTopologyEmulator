#pragma once

#include <variant>

#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AntiWindup/BackCalculationAntiWindupConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AntiWindup/ClampingAntiWindupConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AntiWindup/NoneAntiWindupConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AntiWindup/TrackingModeAntiWindupConfig.h"

namespace starTopologyEmulator
{

using PiAntiWindupConfig = std::variant<
	NoneAntiWindupConfig,
	ClampingAntiWindupConfig,
	BackCalculationAntiWindupConfig,
	TrackingModeAntiWindupConfig>;

} // namespace starTopologyEmulator
