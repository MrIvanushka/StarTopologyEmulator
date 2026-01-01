#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyFactory.h"

#include "StarHubStrategy/SimpleStarHubStrategy.h"

namespace starTopologyEmulator
{

std::unique_ptr<IStarHubStrategy> StarHubStrategyFactory::make(StarHubStrategyConfig config)
{
	return std::make_unique<SimpleStarHubStrategy>(std::move(config));
}

} // namespace starTopologyEmulator
