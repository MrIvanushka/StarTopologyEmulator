#pragma once

#include <deque>
#include <memory>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/PiLoadControllerConfig.h"

namespace starTopologyEmulator
{

class PiLoadController : public IIncomeLoadController
{
public:
	PiLoadController(
		std::shared_ptr<IDynamicFrameSettings>,
		std::shared_ptr<IIncomeStationsPredictor>,
		PiLoadControllerConfig&&,
		MetricScope scope = {});

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;

private:
	double estimateLoad(
		double readyUsers,
		double currentPTx,
		std::uint32_t currentBaseWindow,
		std::uint64_t plannedRaSlots) const;

	double clampProbability(double value) const;
	double clampStep(double delta) const;
	double pushAndIntegrate(double error);

	const PiLoadControllerConfig _config;

	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	std::deque<double> _errorWindow;
	double _errorSum = 0.0;

	StarHubPlanMessage::BackoffConfig _lastOutput;

	MetricScope _scope;
	MetricHandle _hPTx = kInvalidMetricHandle;
	MetricHandle _hError = kInvalidMetricHandle;
	MetricHandle _hLoad = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
