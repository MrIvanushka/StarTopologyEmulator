#pragma once

#include <functional>
#include <map>

#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/Messages/StarStationMessage.h"
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class StarHub : public IStarHub
{
public:
	StarHub(
		std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
		std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
		std::shared_ptr<IFrameCalculator> frameCalculator,
		std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
		std::unique_ptr<IStarHubStrategy> strategy,
		std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
		Timestamp tts,
		MetricScope scope = {});

	void update(Timestamp currentTime) override;

	void handleMessage(std::shared_ptr<IMessage>, Timestamp) override;

	Timestamp tts() const override;
private:
	void onFrameEnd(std::uint64_t frameNumber);

	void sendAnswersToStations(Timestamp);
private:
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> _sendFunc;

	std::shared_ptr<IIncomeLoadEstimator> _incomeLoadEstimator;
	std::shared_ptr<IFrameCalculator> _frameCalculator;
	std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
	std::unique_ptr<IStarHubStrategy> _strategy;

	Timestamp _tts;

	std::vector<StationID> _pendingAnswers;

	RandomAccessFrameResult _frameAccumulator;
	std::uint64_t _lastProcessedFrame = 0;

	std::shared_ptr<IBacklogAccumulator> _backlogAccumulator;

	MetricScope _scope;
	MetricHandle _hPendingAnswers = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
