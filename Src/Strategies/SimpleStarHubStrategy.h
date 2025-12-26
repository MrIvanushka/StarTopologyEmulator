#pragma once

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/StarHubStrategyConfig.h"

namespace starTopologyEmulator
{

class SimpleStarHubStrategy : public IStarHubStrategy
{
public:
	using Config = StarHubStrategyConfig;

	SimpleStarHubStrategy(const Config&& config) : _cfg(std::move(config))
	{ }

	StarHubPlanMessage generate(double g, double plr) override;

private:
	int calculateRaSlots(double g, double plr);

	const Config _cfg;
};

} // namespace starTopologyEmulator
