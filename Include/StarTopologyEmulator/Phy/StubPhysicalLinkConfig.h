#pragma once

#include <cstddef>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StubPhysicalLinkConfig
{
	std::size_t infoBits = 64;
	float decodingSinrThreshold = 4.0f;
};

} // namespace starTopologyEmulator
