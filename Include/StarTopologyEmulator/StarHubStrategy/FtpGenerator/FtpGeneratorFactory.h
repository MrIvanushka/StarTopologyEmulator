#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT FtpGeneratorFactory
{
public:
    static std::unique_ptr<IFtpGenerator> make(
		std::uint8_t raSlotsCount,
		std::uint8_t yellowSlotsCount,
		std::uint8_t operationSlotsCount);
};

} // namespace starTopologyEmulator
