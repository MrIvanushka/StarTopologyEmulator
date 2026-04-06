#include "StarTopologyEmulator/Stations/StarStationFactory.h"

#include "Stations/StarStation.h"

namespace starTopologyEmulator
{

std::shared_ptr<IStarStation> StarStationFactory::make(StarStationInitData&& initData)
{
	return std::make_shared<StarStation>(
		initData.sendFunc,
		std::move(initData.frameCalculator),
		std::move(initData.dynamicFrameSettings),
		initData.id, initData.messagesNeeded,
		initData.tts, initData.rng);
}

} // namespace starTopologyEmulator
