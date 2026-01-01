#include "StarTopologyEmulator/Stations/StarStationFactory.h"

#include "Stations/StarStation.h"

namespace starTopologyEmulator
{

std::shared_ptr<IStarStation> StarStationFactory::make(
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
	std::unique_ptr<IFrameCalculator> frameCalculator,
	StationID id,
	int messagesNeeded,
	Timestamp tts,
	std::mt19937& rng)
{
	return std::make_shared<StarStation>(sendFunc, std::move(frameCalculator), id, messagesNeeded, tts, rng);
}

} // namespace starTopologyEmulator
