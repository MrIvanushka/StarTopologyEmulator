#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IFtpGenerator
{
public:
	virtual ~IFtpGenerator() = default;

	virtual StarHubPlanMessage::FtpConfig generate(
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) = 0;
};

} // namespace starTopologyEmulator
