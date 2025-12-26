#include "StarTopologyEmulator/StarHubStrategyFactory.h"

#include "Strategies/SimpleStarHubStrategy.h"

namespace starTopologyEmulator
{

std::unique_ptr<IStarHubStrategy> StarHubStrategyFactory::make()
{
	return std::make_unique<SimpleStarHubStrategy>();
}

} // namespace starTopologyEmulator
