#pragma once

#include <functional>
#include <map>
#include <memory>

#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"
#include "StarTopologyEmulator/IFaces/IStationStatsCollector.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class Emulator : public IEmulator
{
public:
	using SendFunc = std::function<void(Timestamp, std::shared_ptr<IMessage>)>;

	Emulator(
		std::function<std::shared_ptr<IStarStation>(SendFunc, StationID, std::shared_ptr<IStationStatsCollector>)>,
		std::function<std::shared_ptr<IStarHub>(SendFunc)>,
		std::unique_ptr<IFrameCalculator>,
		std::unique_ptr<IFrameCalculator>,
		int stationCount,
		std::shared_ptr<IMetricSink> metricSink = nullptr);

	void update(Timestamp currentTime) override;

	std::shared_ptr<IStarHub> hub() const override;

	const std::vector<std::shared_ptr<IStarStation>>& stations() const override;

	std::shared_ptr<IMetricSink> metricSink() const override;
private:
	enum class Direction { ToHub, ToStation };

	struct QueuedMessage
	{
		Timestamp deliveryTime;
		Direction direction;
		StationID stationID;
		std::shared_ptr<IMessage> msg;
	};

	SendFunc makeHubSendFunc();

	SendFunc makeStationSendFunc(StationID stationID);

	void enqueueFromHub(
		Timestamp sendTime,
		std::shared_ptr<IMessage> msg);

	void enqueueFromStation(
		StationID stationID,
		Timestamp sendTime,
		std::shared_ptr<IMessage> msg);

	void updateDownlink(Timestamp, const std::vector<QueuedMessage>&);

	void updateUplink(Timestamp, const std::vector<QueuedMessage>&);

	std::uint32_t stationsCountOnState(TerminalState) const;

private:
	void storeInputLoadAndPlr(std::uint64_t completedFrame);

	std::unique_ptr<IFrameCalculator> _abonentFrameCalculator;
	std::unique_ptr<IFrameCalculator> _hubFrameCalculator;

	std::shared_ptr<IStarHub> _hub;
	std::vector<std::shared_ptr<IStarStation>> _stations;

	std::multimap<Timestamp, QueuedMessage> _queue;

	std::uint64_t _lastProcessedFrame{ 0 };
	std::uint64_t _uplinkAttempted{ 0 };
	std::uint64_t _uplinkOk{ 0 };
	std::uint64_t _uplinkLost{ 0 };
	std::uint64_t _uplinkSlotsWithTraffic{ 0 };

	std::shared_ptr<IMetricSink> _metricSink;
	MetricScope _scope;
	MetricHandle _hStationsOperation = kInvalidMetricHandle;
	MetricHandle _hStationsAcquisition = kInvalidMetricHandle;
	MetricHandle _hStationsOff = kInvalidMetricHandle;
	MetricHandle _hIncomeLoad = kInvalidMetricHandle;
	MetricHandle _hPlr = kInvalidMetricHandle;
	std::shared_ptr<IStationStatsCollector> _stationStats;
};

} // namespace starTopologyEmulator
