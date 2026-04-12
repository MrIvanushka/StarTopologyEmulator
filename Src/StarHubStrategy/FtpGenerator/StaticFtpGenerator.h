#pragma once

#include <utility>

#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"

namespace starTopologyEmulator
{

class StaticFtpGenerator : public IFtpGenerator
{
public:
	StaticFtpGenerator(
		std::uint8_t raSlotsCount,
		std::uint8_t yellowSlotsCount,
		std::uint8_t operationSlotsCount);

	StarHubPlanMessage::FtpConfig generate(std::uint64_t frame) override;
private:
	StarHubPlanMessage::FtpConfig _config;
};

} // namespace starTopologyEmulator
