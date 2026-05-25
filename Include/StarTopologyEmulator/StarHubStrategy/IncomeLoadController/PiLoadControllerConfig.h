#pragma once

#include <cstdint>

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AntiWindup/PiAntiWindupConfig.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT PiLoadControllerConfig
{
	double gTarget = 0.9;

	double kP = 0.5;
	double kI = 0.05;
	std::uint32_t integralWindowFrames = 10;

	double alpha = 1.0;

	double minProbability = 1e-3;
	double maxProbability = 1.0;
	double maxProbabilityStep = 1.0;

	StarHubPlanMessage::BackoffConfig backoffTemplate;

	double epsilon = 1e-9;

	PiAntiWindupConfig antiWindup = NoneAntiWindupConfig{};

	bool allowFeedForward = false;
};

} // namespace starTopologyEmulator
