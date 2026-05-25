#include "StarTopologyEmulator/Stations/StarStationFactory.h"

#include "StarTopologyEmulator/TrafficProfile/CbrTrafficProfileConfig.h"
#include "StarTopologyEmulator/TrafficProfile/TrafficProfileFactory.h"
#include "Stations/StarStation.h"

namespace starTopologyEmulator
{

std::shared_ptr<IStarStation> StarStationFactory::make(StarStationInitData&& initData)
{
	return std::make_shared<StarStation>(
		initData.sendFunc,
		std::move(initData.frameCalculator),
		std::move(initData.dynamicFrameSettings),
		TrafficProfileFactory::make(CbrTrafficProfileConfig{}),
		initData.id, initData.messagesNeeded,
		initData.tts, initData.rng,
		std::move(initData.statsCollector));
}

} // namespace starTopologyEmulator
