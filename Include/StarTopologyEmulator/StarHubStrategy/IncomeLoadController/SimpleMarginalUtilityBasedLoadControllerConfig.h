#pragma once

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

// NUM (Network Utility Maximization), раздел 2.3.2.5.
// U(p) = wThroughput * n * p * (1 - p)^(n - 1)
//      - wCollision  * [1 - (1 - p)^n - n * p * (1 - p)^(n - 1)]
// p_{i+1} = clip( p_i + sat(eta * dU/dp, +-maxProbabilityStep), pMin, pMax )
struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT SimpleMarginalUtilityBasedLoadControllerConfig
{
	double weightThroughput = 1.0;
	double weightCollision  = 1.0;

	double gradientStep = 0.1;

	double maxProbabilityStep = 0.2;

	double minProbability = 1e-3;
	double maxProbability = 1.0;

	StarHubPlanMessage::BackoffConfig backoffTemplate;

	double epsilon = 1e-9;
};

} // namespace starTopologyEmulator
