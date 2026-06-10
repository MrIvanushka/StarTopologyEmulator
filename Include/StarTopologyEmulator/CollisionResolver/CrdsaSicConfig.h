#pragma once

#include <cstddef>
#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT CrdsaSicConfig
{
	std::size_t slotsPerFrame = 100;
	std::uint32_t replicaCount = 2;
	double esN0Db = 10.0;
	double powerImbalanceDb = 0.0;
	std::uint32_t maxSicIterations = 16;
	std::uint64_t seed = 0xC0FFEEULL;
};

} // namespace starTopologyEmulator
