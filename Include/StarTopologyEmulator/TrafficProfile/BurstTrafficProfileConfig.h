#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT BurstTrafficProfileConfig
{
	double packetsPerTimestampOn = 0.0;
	std::uint64_t bitsPerPacket = 0;
	double meanOnDuration = 0.0;
	double meanOffDuration = 0.0;
	std::uint32_t seed = 0;
};

} // namespace starTopologyEmulator
