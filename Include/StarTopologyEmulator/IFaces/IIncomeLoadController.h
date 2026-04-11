#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IMetricProducer.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IIncomeLoadController : public IMetricProducer
{
public:
	virtual StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) = 0;
};

} // namespace starTopologyEmulator
