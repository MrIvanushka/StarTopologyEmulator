#include "StarTopologyEmulator/StarHubStrategyFactory.h"

#include "Strategies/SimpleStarHubStrategy.h"

namespace starTopologyEmulator
{

std::unique_ptr<IStarHubStrategy> StarHubStrategyFactory::make(const StarHubStrategyConfig&& config)
{
	return std::make_unique<SimpleStarHubStrategy>(std::move(config));
}

} // namespace starTopologyEmulator
