#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT Aff3ctPhysicalLinkConfig
{
	std::string hMatrixPath;
	std::size_t infoBits = 0;
	std::size_t crcSize = 16;
	std::string crcPolynomial = "16-CCITT";
	std::uint32_t bpIterations = 50;
};

} // namespace starTopologyEmulator
