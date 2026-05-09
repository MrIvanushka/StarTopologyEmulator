#pragma once

#include <cstdint>

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT GreyModelAdaptiveBackoffControllerConfig
{
	double gTarget = 0.9;

	std::uint32_t historySize = 6;
	std::uint32_t minHistoryForPrediction = 4;

	std::uint8_t minBackoffWindow = 1;
	std::uint8_t maxBackoffWindow = 64;

	StarHubPlanMessage::BackoffConfig backoffTemplate;

	double epsilon = 1e-9;
};

} // namespace starTopologyEmulator
