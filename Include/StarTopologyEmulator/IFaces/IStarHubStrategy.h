#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IStarHubStrategy
{
public:
	virtual ~IStarHubStrategy() = default;

	virtual std::shared_ptr<StarHubPlanMessage> generate(std::uint64_t currentFrame, std::uint64_t targetFrame) = 0;
};

} // namespace starTopologyEmulator
