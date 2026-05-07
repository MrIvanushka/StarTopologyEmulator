#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/SimpleMarginalUtilityBasedLoadControllerConfig.h"

namespace starTopologyEmulator
{

class SimpleMarginalUtilityBasedLoadController : public IIncomeLoadController
{
public:
	SimpleMarginalUtilityBasedLoadController(
		std::shared_ptr<IDynamicFrameSettings>,
		std::shared_ptr<IIncomeStationsPredictor>,
		SimpleMarginalUtilityBasedLoadControllerConfig&&,
		MetricScope scope = {});

	StarHubPlanMessage::BackoffConfig generate(
		std::uint64_t plannedRaSlots,
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;

private:
	double clampProbability(double value) const;

	double utilityGradient(double txProbability, double estimatedStations) const;

private:
	const SimpleMarginalUtilityBasedLoadControllerConfig _config;

	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
	std::shared_ptr<IIncomeStationsPredictor> _readyUsersPredictor;

	MetricScope _scope;
	MetricHandle _hPTx = kInvalidMetricHandle;
	MetricHandle _hBackoff = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
