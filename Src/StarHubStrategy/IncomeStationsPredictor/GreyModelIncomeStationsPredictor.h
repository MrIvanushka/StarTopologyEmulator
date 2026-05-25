#pragma once

#include <optional>
#include <unordered_map>
#include <utility>

#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeStationsPredictor/GreyModelIncomeStationsPredictorConfig.h"

namespace starTopologyEmulator
{

class GreyModelIncomeStationsPredictor : public IIncomeStationsPredictor
{
public:
	GreyModelIncomeStationsPredictor(
		std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
		std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
		GreyModelIncomeStationsPredictorConfig&& config,
		MetricScope scope = {});

	double estimateReadyUsers(
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;
private:
	double clampProbability(double value) const;

	double backoffEligibilityFactor(std::uint32_t windowFrames) const;

	double effectiveAggressiveness(
		double txProbability,
		std::uint32_t backoffWindowFrames) const;

	std::optional<double> historyReadyUsers(
		const std::shared_ptr<StarHubPlanMessage>& plan) const;
private:
	const GreyModelIncomeStationsPredictorConfig _config;

	std::unordered_map<std::uint64_t, double> _incomeLoadHistory;
	std::shared_ptr<IIncomeLoadEstimator> _incomeLoadEstimator;
	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;

	MetricScope _scope;
	MetricHandle _hReadyUsers = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
