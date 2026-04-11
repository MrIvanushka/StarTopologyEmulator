#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT TargetLoadControllerConfig
{
	// Целевая входная нагрузка.
	double gTarget = 0.80;

	// Мёртвая зона вокруг целевой нагрузки:
	// если прогноз уже внутри [gTarget - gDeadBand, gTarget + gDeadBand],
	// новое управление не пересчитывается.
	double gDeadBand = 0.05;

	// Ограничения и сглаживание вероятности вещания.
	double minProbability = 0.01;
	double maxProbability = 1.0;
	double alphaProbability = 1.0;
	double maxProbabilityStep = 1.0;

	// Ограничения и сглаживание окна backoff.
	std::uint32_t minBackoffWindowFrames = 1;
	std::uint32_t maxBackoffWindowFrames = 256;
	double alphaBackoff = 1.0;
	std::uint32_t maxBackoffStepFrames = 256;

	double epsilon = 1e-9;
};

} // namespace starTopologyEmulator
