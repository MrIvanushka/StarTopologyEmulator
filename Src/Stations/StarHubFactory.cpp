#include "StarTopologyEmulator/Stations/StarHubFactory.h"

#include "Stations/StarHub.h"

namespace starTopologyEmulator
{

std::shared_ptr<IStarHub> StarHubFactory::make(StarHubInitData&& initData)
{
	return std::make_shared<StarHub>(
		initData.sendFunc,
		std::move(initData.incomeLoadEstimator),
		std::move(initData.frameCalculator),
		std::move(initData.strategy),
		std::move(initData.dynamicFrameSettings),
		initData.tts);
}

} // namespace starTopologyEmulator
