#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IDynamicFrameSettings
{
public:
	virtual ~IDynamicFrameSettings() = default;

	virtual void handlePlan(std::shared_ptr<StarHubPlanMessage>) = 0;

	virtual void clearOutdated(std::uint64_t frame) = 0;

	virtual std::shared_ptr<StarHubPlanMessage> currentPlan(std::uint64_t frame) const = 0;

	virtual std::uint64_t earliestPlanNumber() const = 0;
};

} // namespace starTopologyEmulator
