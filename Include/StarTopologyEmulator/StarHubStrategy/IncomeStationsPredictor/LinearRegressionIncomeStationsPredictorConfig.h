#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT LinearRegressionIncomeStationsPredictorConfig
{
	std::uint32_t regressionWindow = 16;

	double forgettingHorizonSec = 2.0;

	double minProbability = 0.01;
	double maxProbability = 1.0;

	double epsilon = 1e-9;
};


} // namespace starTopologyEmulator
