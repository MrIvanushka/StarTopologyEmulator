#pragma once

#include <cstdint>
#include <vector>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT SimpleMarginalUtilityBasedLoadControllerConfig
{
	// Веса исправленной функции полезности:
	// U = alpha * success - beta * collision - gamma * delayPenalty
	double alphaSuccess = 1.0;
	double betaCollision = 1.0;
	double gammaDelay = 1.0;

	// Нормировочный предел по задержке в кадрах.
	double tMaxFrames = 20.0;

	// Если улучшение полезности меньше этого порога, управление не меняется.
	double minUtilityGain = 1e-6;

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

	// Шаг перебора вероятности вещания.
	double probabilityGridStep = 0.05;

	// Набор кандидатов для окна backoff.
	std::vector<std::uint32_t> backoffCandidates =
	{
		1, 2, 4, 8, 16, 32, 64, 128, 256
	};

	double epsilon = 1e-9;
};

} // namespace starTopologyEmulator
