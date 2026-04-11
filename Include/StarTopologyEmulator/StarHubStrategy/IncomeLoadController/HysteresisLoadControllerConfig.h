#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT HysteresisLoadControllerConfig
{
	// Пороги гистерезиса для прогнозной нагрузки.
	double gLow = 0.70;
	double gHigh = 0.95;

	// Параметры режима Normal.
	double pNormal = 1.0;
	std::uint32_t backoffNormalFrames = 1;

	// Параметры режима Critical.
	double pCritical = 0.25;
	std::uint32_t backoffCriticalFrames = 16;

	// Ограничения и сглаживание выхода регулятора.
	double minProbability = 0.01;
	double maxProbability = 1.0;
	double alphaProbability = 1.0;
	double maxProbabilityStep = 1.0;

	std::uint32_t minBackoffWindowFrames = 1;
	std::uint32_t maxBackoffWindowFrames = 256;
	double alphaBackoff = 1.0;
	std::uint32_t maxBackoffStepFrames = 256;

	double epsilon = 1e-9;
};

} // namespace starTopologyEmulator
