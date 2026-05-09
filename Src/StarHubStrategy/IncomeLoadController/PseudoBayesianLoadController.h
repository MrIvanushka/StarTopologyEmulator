#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/PseudoBayesianLoadControllerConfig.h"

namespace starTopologyEmulator
{

class PseudoBayesianLoadController : public IIncomeLoadController
{
public:
	PseudoBayesianLoadController(
		std::shared_ptr<IIncomeStationsPredictor>,
		PseudoBayesianLoadControllerConfig&&,
		MetricScope scope = {});

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;

private:
	const PseudoBayesianLoadControllerConfig _config;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	MetricScope _scope;
	MetricHandle _hPTx = kInvalidMetricHandle;
	MetricHandle _hN = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
