#pragma once

#include <utility>

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyConfig.h"

namespace starTopologyEmulator
{

class SimpleStarHubStrategy : public IStarHubStrategy
{
public:
	SimpleStarHubStrategy(
		std::shared_ptr<IIncomeLoadEstimator>,
		StarHubStrategyConfig&&,
		MetricScope scope = {});

	std::shared_ptr<StarHubPlanMessage> generate(std::uint64_t currentFrame, std::uint64_t targetFrame) override;

private:
	int calculateRaSlots(double g, double plr);

	const StarHubStrategyConfig _cfg;

	std::shared_ptr<IIncomeLoadEstimator> _incomeLoadEstimator;

	MetricScope _scope;
	MetricHandle _hBaseWindow = kInvalidMetricHandle;
	MetricHandle _hMaxWindow = kInvalidMetricHandle;
	MetricHandle _hPTx = kInvalidMetricHandle;
	MetricHandle _hRaSlots = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
