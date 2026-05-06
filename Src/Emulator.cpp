#include "Emulator.h"

#include "StarTopologyEmulator/Messages/CollisionReport.h"
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"

namespace starTopologyEmulator
{

Emulator::Emulator(
	std::function<std::shared_ptr<IStarStation>(SendFunc, StationID)> stationFactory,
	std::function<std::shared_ptr<IStarHub>(SendFunc)> hubFactory,
	std::unique_ptr<IFrameCalculator> frameCalculator,
	int stationCount)
{
	_stations.resize(stationCount);
	_frameCalculator = std::move(frameCalculator);

	for (auto i = 0u; i < stationCount; ++i)
		_stations[i] = stationFactory(makeStationSendFunc(i), i);

	_hub = hubFactory(makeHubSendFunc());

	REGISTER_METRIC_SUBFOLDER(_hub.get());
	REGISTER_METRIC(stationsCountOnState(TerminalState::OPERATION), "���������� �������� � ���� �������");
	REGISTER_METRIC(stationsCountOnState(TerminalState::ACQUISITION), "�������� �������");
	REGISTER_METRIC(stationsCountOnState(TerminalState::OFF), "������������� �������");
	REGISTER_METRIC(_previousFrameIncomeLoad, "�������� ������� ��������");
	REGISTER_METRIC(_previousFramePlr, "�������� PLR");
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

void Emulator::update(Timestamp now)
{
	auto currentFrameMoment = _frameCalculator->frameMoment(now);
	_uplinkSlotsWithTraffic = currentFrameMoment.frameNumber * _frameCalculator->frameConfig().slotCountInFrame + currentFrameMoment.slotNumber;

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
		storeInputLoadAndPlr();
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
	_uplinkAttempted += 1;

	QueuedMessage queuedMessage;
	queuedMessage.deliveryTime = sendTime + _stations[stationID]->tts() + _hub->tts();
	queuedMessage.direction = Direction::ToHub;
	queuedMessage.stationID = stationID;
	queuedMessage.msg = msg;

	auto deliveryFrameSlot = _frameCalculator->frameMoment(queuedMessage.deliveryTime);
	auto deliverySlotBegin = _frameCalculator->slotBeginTime(deliveryFrameSlot.frameNumber, deliveryFrameSlot.slotNumber);
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
	Timestamp deliveryTime = 0;
	std::map<Timestamp, std::vector<std::shared_ptr<IMessage>>> msgs;

	for (auto& message : releasedMessages)
	{
		if (message.direction != Direction::ToHub)
			continue;

		if (message.msg->isCollisionImmune())
		{
			_hub->handleMessage(message.msg, message.deliveryTime);
			continue;
		}

		msgs[message.deliveryTime].push_back(message.msg);
	}
	for (auto pair : msgs)
	{
		if (pair.second.size() == 1)
		{
			_uplinkOk += static_cast<std::uint64_t>(msgs.size());
			_hub->handleMessage(pair.second[0], pair.first);
		}
		else if (pair.second.size() > 1)
		{
			_uplinkLost += static_cast<std::uint64_t>(msgs.size());
			_hub->handleMessage(std::make_shared<CollisionReport>(), pair.first);
		}
	}
}

std::uint32_t Emulator::stationsCountOnState(TerminalState state) const
{
	std::uint32_t ret = 0;
	for (auto& station : _stations)
		if (station && station->currentState() == state)
			++ret;
	return ret;
}

void Emulator::storeInputLoadAndPlr()
{
	if (_uplinkSlotsWithTraffic == 0)
	{
		_previousFramePlr = 0;
		_previousFrameIncomeLoad = 0;
	}
	else
	{
		_previousFrameIncomeLoad = static_cast<double>(_uplinkAttempted) / static_cast<double>(_frameCalculator->frameConfig().slotCountInFrame / 2);
		_previousFramePlr = static_cast<double>(_uplinkLost) / static_cast<double>(_uplinkAttempted);
	}
	_uplinkAttempted = 0;
	_uplinkSlotsWithTraffic = 0;
}

} // namespace starTopologyEmulator
