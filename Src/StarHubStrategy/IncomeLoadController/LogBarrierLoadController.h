#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/LogBarrierLoadControllerConfig.h"

namespace starTopologyEmulator
{

class LogBarrierLoadController : public IIncomeLoadController
{
public:
	LogBarrierLoadController(
		std::shared_ptr<IDynamicFrameSettings>,
		std::shared_ptr<IIncomeStationsPredictor>,
		LogBarrierLoadControllerConfig&&,
		MetricScope scope = {});

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;

private:
	double clampProbability(double value) const;
	double clampStep(double delta) const;
	double utilityGradient(double pTx, double n) const;

	const LogBarrierLoadControllerConfig _config;

	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	MetricScope _scope;
	MetricHandle _hPTx = kInvalidMetricHandle;
	MetricHandle _hBackoff = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
