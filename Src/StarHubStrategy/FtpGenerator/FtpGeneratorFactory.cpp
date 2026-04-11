#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/FtpGeneratorFactory.h"

#include "StaticFtpGenerator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::make(
	std::uint8_t raSlotsCount,
	std::uint8_t yellowSlotsCount,
	std::uint8_t operationSlotsCount)
{
	return std::make_unique<StaticFtpGenerator>(raSlotsCount, yellowSlotsCount, operationSlotsCount);
}

} // namespace starTopologyEmulator
