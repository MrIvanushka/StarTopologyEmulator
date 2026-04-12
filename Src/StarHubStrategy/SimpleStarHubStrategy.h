#pragma once

#include <utility>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyConfig.h"

namespace starTopologyEmulator
{

class SimpleStarHubStrategy : public IStarHubStrategy
{
	DECLARE_METRICS("Стратегия случайного доступа")
public:
	SimpleStarHubStrategy(
		std::shared_ptr<IIncomeLoadEstimator>,
		StarHubStrategyConfig&&);

	std::shared_ptr<StarHubPlanMessage> generate(std::uint64_t currentFrame, std::uint64_t targetFrame) override;

private:
	int calculateRaSlots(double g, double plr);

	const StarHubStrategyConfig _cfg;

	std::shared_ptr<IIncomeLoadEstimator> _incomeLoadEstimator;

	double _baseWindow = 0;
	double _maxWindow = 0;
	double _pTx = 0;
	int _raSlotsCount = 0;
};

} // namespace starTopologyEmulator
