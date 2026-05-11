#pragma once

#include <unordered_map>

#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class BackoffAwareCogorthyIncomeStationsPredictor : public IIncomeStationsPredictor
{
public:
	BackoffAwareCogorthyIncomeStationsPredictor(
		std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
		std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
		MetricScope scope = {});

	double estimateReadyUsers(
		std::uint64_t currentFrame,
		std::uint64_t targetFrame) override;

private:
	double calculateImpact(
		std::uint64_t impactFrame,
		std::uint64_t targetFrame,
		std::uint64_t commandApplyDelay);

	std::unordered_map<std::uint64_t, double> _incomeLoadHistory;
	std::shared_ptr<IIncomeLoadEstimator> _incomeLoadEstimator;
	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;

	MetricScope _scope;
	MetricHandle _hReadyUsers = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
