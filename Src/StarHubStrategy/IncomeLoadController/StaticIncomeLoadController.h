#pragma once

#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"

namespace starTopologyEmulator
{

class StaticIncomeLoadController : public IIncomeLoadController
{
public:
	StaticIncomeLoadController(StarHubPlanMessage::BackoffConfig);

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;
private:
	StarHubPlanMessage::BackoffConfig _config;
};

} // namespace starTopologyEmulator
