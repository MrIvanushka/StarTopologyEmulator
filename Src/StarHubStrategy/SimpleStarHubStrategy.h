#pragma once

#include <utility>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyConfig.h"

namespace starTopologyEmulator
{

class SimpleStarHubStrategy : public IStarHubStrategy
{
	DECLARE_METRICS("Стратегия случайного доступа")
public:
	using Config = StarHubStrategyConfig;

	SimpleStarHubStrategy(Config config);

	std::shared_ptr<StarHubPlanMessage> generate(double g, double plr) override;

private:
	int calculateRaSlots(double g, double plr);

	const Config _cfg;

	double _baseWindow = 0;
	double _maxWindow = 0;
	double _pTx = 0;
	int _raSlotsCount = 0;
};

} // namespace starTopologyEmulator
