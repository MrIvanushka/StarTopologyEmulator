#include "StarTopologyEmulator/Stations/StarHubFactory.h"

#include "Stations/StarHub.h"

namespace starTopologyEmulator
{

std::shared_ptr<IStarHub> StarHubFactory::make(StarHubInitData&& initData)
{
	return std::make_shared<StarHub>(
		initData.sendFunc,
		initData.incomeLoadEstimator,
		initData.frameCalculator,
		initData.dynamicFrameSettings,
		std::move(initData.strategy),
		std::move(initData.backlogAccumulator),
		initData.tts);
}

} // namespace starTopologyEmulator
