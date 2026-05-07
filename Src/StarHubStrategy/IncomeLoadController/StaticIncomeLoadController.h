#pragma once

#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class StaticIncomeLoadController : public IIncomeLoadController
{
public:
	StaticIncomeLoadController(
		std::shared_ptr<IIncomeStationsPredictor>,
		StarHubPlanMessage::BackoffConfig,
		MetricScope scope = {});

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;
private:
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	StarHubPlanMessage::BackoffConfig _config;

	MetricScope _scope;
	MetricHandle _hPTx = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
