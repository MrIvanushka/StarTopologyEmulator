#pragma once

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"

namespace starTopologyEmulator
{

class StaticIncomeLoadController : public IIncomeLoadController
{
	DECLARE_METRICS("Статический контроллер входной нагрузки");
public:
	StaticIncomeLoadController(
		std::shared_ptr<IIncomeStationsPredictor>,
		StarHubPlanMessage::BackoffConfig);

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;
private:
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	StarHubPlanMessage::BackoffConfig _config;
};

} // namespace starTopologyEmulator
