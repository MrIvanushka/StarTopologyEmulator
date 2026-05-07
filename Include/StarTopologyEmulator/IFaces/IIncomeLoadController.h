#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IIncomeLoadController
{
public:
	virtual ~IIncomeLoadController() = default;

	virtual StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) = 0;
};

} // namespace starTopologyEmulator
