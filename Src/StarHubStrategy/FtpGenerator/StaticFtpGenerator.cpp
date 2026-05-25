#include "StaticFtpGenerator.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

StaticFtpGenerator::StaticFtpGenerator(
	std::uint8_t raSlotsCount,
	std::uint8_t yellowSlotsCount,
	std::uint8_t operationSlotsCount)
	: _config{ operationSlotsCount, yellowSlotsCount, raSlotsCount }
{ }

StarHubPlanMessage::FtpConfig StaticFtpGenerator::generate(
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	return _config;
}

} // namespace starTopologyEmulator
