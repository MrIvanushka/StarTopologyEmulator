#pragma once

#include <deque>
#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/GreyModelAdaptiveBackoffControllerConfig.h"

namespace starTopologyEmulator
{

class GreyModelAdaptiveBackoffController : public IIncomeLoadController
{
public:
	GreyModelAdaptiveBackoffController(
		std::shared_ptr<IIncomeStationsPredictor>,
		GreyModelAdaptiveBackoffControllerConfig&&,
		MetricScope scope = {});

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;

private:
	void pushObservation(double value);
	double predictNext() const;
	std::uint8_t selectBackoff(
		double predictedReadyUsers,
		std::uint64_t plannedRaSlots) const;

	const GreyModelAdaptiveBackoffControllerConfig _config;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	std::deque<double> _history;

	MetricScope _scope;
	MetricHandle _hPTx = kInvalidMetricHandle;
	MetricHandle _hBackoff = kInvalidMetricHandle;
	MetricHandle _hPrediction = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
