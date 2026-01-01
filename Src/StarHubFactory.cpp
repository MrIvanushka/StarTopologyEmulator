#include "StarTopologyEmulator/StarHubFactory.h"

#include "Stations/StarHub.h"

namespace starTopologyEmulator
{

std::shared_ptr<IStarHub> StarHubFactory::make(
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
	std::unique_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::unique_ptr<IFrameCalculator> frameCalculator,
	std::unique_ptr<IStarHubStrategy> strategy,
	Timestamp tts)
{
	return std::make_shared<StarHub>(sendFunc,
		std::move(incomeLoadEstimator), std::move(frameCalculator),
		std::move(strategy), tts);
}

} // namespace starTopologyEmulator
