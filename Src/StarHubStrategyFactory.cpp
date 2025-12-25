#include "StarTopologyEmulator/StarHubFactory.h"

#include "Stations/StarHub.h"

namespace starTopologyEmulator
{

std::shared_ptr<IStarHub> StarHubFactory::make(
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
	const std::vector<StationID>& stations,
	std::unique_ptr<IStarHubStrategy> strategy)
{
	return std::make_shared<StarHub>(sendFunc, stations, std::move(strategy));
}

} // namespace starTopologyEmulator
