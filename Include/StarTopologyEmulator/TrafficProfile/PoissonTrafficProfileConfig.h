#pragma once

#include <cstdint>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT PoissonTrafficProfileConfig
{
	// λ from § 2.1.4.1: mean number of packet arrivals per Timestamp unit.
	double packetsPerTimestamp = 0.0;
	std::uint64_t bitsPerPacket = 0;
	// 0 ⇒ nondeterministic seed via std::random_device.
	std::uint32_t seed = 0;
};

} // namespace starTopologyEmulator
