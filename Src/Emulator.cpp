#include "Emulator.h"

#include "StarTopologyEmulator/Messages/CollisionReport.h"
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"

namespace starTopologyEmulator
{

Emulator::Emulator(
	std::function<std::shared_ptr<IStarStation>(SendFunc, StationID)> stationFactory,
	std::function<std::shared_ptr<IStarHub>(SendFunc)> hubFactory,
	int stationCount)
{
	_stations.resize(stationCount);

	for (auto i = 0u; i < stationCount; ++i)
		_stations[i] = stationFactory(makeStationSendFunc(i), i);

	_hub = hubFactory(makeHubSendFunc());

	REGISTER_METRIC_SUBFOLDER(_hub.get());
	REGISTER_METRIC(joinedStationsCount(), "Количество вошедших в сеть станций");
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
	std::vector<QueuedMessage> releasedMessages;

	while (!_queue.empty() && _queue.begin()->first <= now)
	{
		releasedMessages.push_back(std::move(_queue.begin()->second));
		_queue.erase(_queue.begin());
	}

	updateDownlink(now, releasedMessages);
	updateUplink(now, releasedMessages);
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
			station->handleMessage(message.msg, now);
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

		msgs[message.deliveryTime].push_back(message.msg);
	}
	for (auto pair : msgs)
	{
		if (pair.second.size() == 1)
			_hub->handleMessage(pair.second[0], pair.first);
		else if (msgs.size() > 1)
			_hub->handleMessage(std::make_shared<CollisionReport>(), pair.first);
	}
}

std::uint32_t Emulator::joinedStationsCount() const
{
	std::uint32_t ret = 0;
	for (auto& station : _stations)
		if (station && station->joinedTime().has_value())
			++ret;
	return ret;
}

} // namespace starTopologyEmulator
