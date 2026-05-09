#pragma once

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT PseudoBayesianLoadControllerConfig
{
	double minProbability = 1e-3;
	double maxProbability = 1.0;

	StarHubPlanMessage::BackoffConfig backoffTemplate;

	double epsilon = 1e-9;
};

} // namespace starTopologyEmulator
