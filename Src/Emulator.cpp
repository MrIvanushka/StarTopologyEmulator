#include "Emulator.h"

#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"
#include "StarTopologyEmulator/Metrics/StationStatsCollector.h"

namespace starTopologyEmulator
{

Emulator::Emulator(
	std::function<std::shared_ptr<IStarStation>(SendFunc, StationID, std::shared_ptr<IStationStatsCollector>)> stationFactory,
	std::function<std::shared_ptr<IStarHub>(SendFunc)> hubFactory,
	std::unique_ptr<IFrameCalculator> abonentFrameCalculator,
	std::unique_ptr<IFrameCalculator> hubFrameCalculator,
	std::unique_ptr<ICollisionResolver> collisionResolver,
	int stationCount,
	std::shared_ptr<IMetricSink> metricSink)
	: _metricSink(std::move(metricSink))
{
	_stations.resize(stationCount);
	_abonentFrameCalculator = std::move(abonentFrameCalculator);
	_hubFrameCalculator = std::move(hubFrameCalculator);
	_collisionResolver = std::move(collisionResolver);

	if (_metricSink)
	{
		_scope = MetricScope(_metricSink, "Эмулятор");
		_hStationsOperation = _scope.registerMetric("Количество станций в фазе работы");
		_hStationsAcquisition = _scope.registerMetric("Входящие станции");
		_hStationsOff = _scope.registerMetric("Выключенные станции");
		_hIncomeLoad = _scope.registerMetric("Число вещаний в RA-сегменте в кадре");
		_hPlr = _scope.registerMetric("Реальный PLR");
		_stationStats = std::make_shared<StationStatsCollector>(_scope.child("Станции"));
	}

	for (auto i = 0u; i < stationCount; ++i)
		_stations[i] = stationFactory(makeStationSendFunc(i), i, _stationStats);

	_hub = hubFactory(makeHubSendFunc());
}

std::function<void(Emulator::Timestamp, std::shared_ptr<IMessage>)> Emulator::makeHubSendFunc()
{
	return [this](Timestamp t, std::shared_ptr<IMessage> msg) { enqueueFromHub(t, std::move(msg)); };
}

std::function<void(Emulator::Timestamp, std::shared_ptr<IMessage>)> Emulator::makeStationSendFunc(StationID stationID)
{
	return [this, stationID](Timestamp t, std::shared_ptr<IMessage> msg) { enqueueFromStation(stationID, t, std::move(msg)); };
}

std::shared_ptr<IStarHub> Emulator::hub() const
{
	return _hub;
}

const std::vector<std::shared_ptr<IStarStation>>& Emulator::stations() const
{
	return _stations;
}

std::shared_ptr<IMetricSink> Emulator::metricSink() const
{
	return _metricSink;
}

void Emulator::update(Timestamp now)
{
	auto currentFrameMoment = _abonentFrameCalculator->frameMoment(now);
	auto hubFrameMoment = _hubFrameCalculator->frameMoment(now);
	_uplinkSlotsWithTraffic = currentFrameMoment.frameNumber * _abonentFrameCalculator->frameConfig().slotCountInFrame + currentFrameMoment.slotNumber;

	std::vector<QueuedMessage> releasedMessages;

	while (!_queue.empty() && _queue.begin()->first <= now)
	{
		releasedMessages.push_back(std::move(_queue.begin()->second));
		_queue.erase(_queue.begin());
	}

	updateDownlink(now, releasedMessages);
	updateUplink(now, releasedMessages);

	if (currentFrameMoment.frameNumber != _lastProcessedFrame)
	{
		storeInputLoadAndPlr(_lastProcessedFrame);
		_lastProcessedFrame = currentFrameMoment.frameNumber;
	}
}

void Emulator::enqueueFromHub(Timestamp sendTime, std::shared_ptr<IMessage> msg)
{
	for (auto station : _stations)
	{
		QueuedMessage queuedMessage;
		queuedMessage.deliveryTime = sendTime + _hub->tts() + station->tts();
		queuedMessage.direction = Direction::ToStation;
		queuedMessage.stationID = station->id();
		queuedMessage.msg = msg;
		_queue.emplace(queuedMessage.deliveryTime, queuedMessage);
	}
}

void Emulator::enqueueFromStation(
	StationID stationID,
	Timestamp sendTime,
	std::shared_ptr<IMessage> msg)
{
	if (!msg->isCollisionImmune())
		_uplinkAttempted += 1;

	QueuedMessage queuedMessage;
	queuedMessage.deliveryTime = sendTime + _stations[stationID]->tts() + _hub->tts();
	queuedMessage.direction = Direction::ToHub;
	queuedMessage.stationID = stationID;
	queuedMessage.msg = msg;

	auto deliveryFrameSlot = _hubFrameCalculator->frameMoment(queuedMessage.deliveryTime);
	auto deliverySlotBegin = _hubFrameCalculator->slotBeginTime(deliveryFrameSlot.frameNumber, deliveryFrameSlot.slotNumber);
	_queue.emplace(deliverySlotBegin, queuedMessage);
}

void Emulator::updateDownlink(Timestamp now, const std::vector<QueuedMessage>& releasedMessages)
{
	for (const auto& message : releasedMessages)
	{
		if (message.direction != Direction::ToStation)
			continue;

		auto station = _stations[message.stationID];
		if (station)
			station->handleMessage(message.msg, message.deliveryTime);
	}
}

void Emulator::updateUplink(Timestamp now, const std::vector<QueuedMessage>& releasedMessages)
{
	std::vector<ICollisionResolver::SlotTransmission> transmissions;

	for (const auto& message : releasedMessages)
	{
		if (message.direction != Direction::ToHub)
			continue;

		if (message.msg->isCollisionImmune())
		{
			_hub->handleMessage(message.msg, message.deliveryTime);
			continue;
		}

		const auto moment = _hubFrameCalculator->frameMoment(message.deliveryTime);
		transmissions.push_back(
			{ static_cast<starTopologyEmulator::Timestamp>(message.deliveryTime), moment.slotNumber, message.msg });
	}

	auto outcome = _collisionResolver->resolveFrame(std::move(transmissions));
	_uplinkOk += outcome.okCount;
	_uplinkLost += outcome.lostCount;
	for (auto& delivered : outcome.toHub)
		_hub->handleMessage(delivered.msg, delivered.deliveryTime);
}

std::uint32_t Emulator::stationsCountOnState(TerminalState state) const
{
	std::uint32_t ret = 0;
	for (auto& station : _stations)
		if (station && station->currentState() == state)
			++ret;
	return ret;
}

void Emulator::storeInputLoadAndPlr(std::uint64_t completedFrame)
{
	const double incomeLoad = static_cast<double>(_uplinkAttempted);
	double plr = 0.0;
	if (_uplinkAttempted != 0)
		plr = static_cast<double>(_uplinkLost) / static_cast<double>(_uplinkAttempted);

	_scope.emit(_hStationsOperation, completedFrame,
		static_cast<double>(stationsCountOnState(TerminalState::OPERATION)));
	_scope.emit(_hStationsAcquisition, completedFrame,
		static_cast<double>(stationsCountOnState(TerminalState::ACQUISITION)));
	_scope.emit(_hStationsOff, completedFrame,
		static_cast<double>(stationsCountOnState(TerminalState::OFF)));
	_scope.emit(_hIncomeLoad, completedFrame, incomeLoad);
	_scope.emit(_hPlr, completedFrame, plr);

	_uplinkAttempted = 0;
	_uplinkOk = 0;
	_uplinkLost = 0;
	_uplinkSlotsWithTraffic = 0;
}

} // namespace starTopologyEmulator
