#pragma once

#include <functional>
#include <map>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/Messages/StarStationMessage.h"
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"

namespace starTopologyEmulator
{

class StarHub : public IStarHub
{
	DECLARE_METRICS("Центральная станция")
public:
	StarHub(
		std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
		std::unique_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
		std::unique_ptr<IFrameCalculator> frameCalculator,
		std::unique_ptr<IStarHubStrategy> strategy,
		std::unique_ptr<IDynamicFrameSettings> dynamicFrameSettings,
		Timestamp tts);

	void update(Timestamp currentTime) override;

	void handleMessage(std::shared_ptr<IMessage>, Timestamp) override;

	Timestamp tts() const override;
private:
	void onFrameEnd(std::uint64_t frameNumber);

	void sendAnswersToStations(Timestamp);
private:
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> _sendFunc;
	
	std::unique_ptr<IIncomeLoadEstimator> _incomeLoadEstimator;
	std::unique_ptr<IFrameCalculator> _frameCalculator;
	std::unique_ptr<IStarHubStrategy> _strategy;
	std::unique_ptr<IDynamicFrameSettings> _dynamicFrameSetings;

	Timestamp _tts;

	std::vector<StationID> _pendingAnswers;

	RandomAccessFrameResult _frameAccumulator;
	std::uint64_t _lastProcessedFrame = 0;
};

} // namespace starTopologyEmulator
