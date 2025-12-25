#include "Emulator.h"

#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"

namespace starTopologyEmulator
{

Emulator::Emulator(
	std::function<std::shared_ptr<IStarStation>(SendFunc, StationID, RTT)> stationFactory,
	std::function<std::shared_ptr<IStarHub>(SendFunc, const std::vector<StationID>&)> hubFactory,
	int stationCount, int rttSlots)
	: _rttSlots(rttSlots)
{
	std::vector<StationID> abonentIDs(stationCount);
	_stations.resize(stationCount);

	for (auto i = 0u; i < stationCount; ++i)
	{
		_stations[i] = stationFactory(makeStationSendFunc(i), i, _rttSlots);
		abonentIDs[i] = i;
	}

	_hub = hubFactory(makeHubSendFunc(), abonentIDs);
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

void Emulator::update(Timestamp currentTime)
{
	auto range = _queue.equal_range(currentTime);
	for (auto it = range.first; it != range.second; ++it)
	{
		const QueuedMessage& queuedMessage = it->second;
		if (queuedMessage.direction == Direction::ToHub)
		{
			if(_hub)
				_hub->handleMessage(queuedMessage.msg, currentTime);
		}
		else
		{
			for (auto& station : _stations)
			{
				if (station->id() == queuedMessage.stationID)
				{
					station->handleMessage(queuedMessage.msg, currentTime);
					break;
				}
			}
		}
	}
	_queue.erase(range.first, range.second);

	for (auto& station : _stations)
	{
		TerminalState state = station->currentState();
		_stationStats[station->id()][state]++;
	}
}

const std::map<StationID, Emulator::StateCounter>& Emulator::stationStats() const
{
	return _stationStats;
}

void Emulator::enqueueFromHub(Timestamp sendTime, std::shared_ptr<IMessage> msg)
{
	if (!msg)
		return;

	if (msg->type() == MessageType::StarHubPlan)
	{
		for (auto& station : _stations)
		{
			QueuedMessage queuedMessage;
			queuedMessage.deliveryTime = sendTime + _rttSlots;
			queuedMessage.direction = Direction::ToStation;
			queuedMessage.stationID = station->id();
			queuedMessage.msg = msg;
			_queue.emplace(queuedMessage.deliveryTime, queuedMessage);
		}
	}
	else if (msg->type() == MessageType::StarHubAccess)
	{
		auto ack = std::static_pointer_cast<StarHubAccessMessage>(msg);
		QueuedMessage queuedMessage;
		queuedMessage.deliveryTime = sendTime + _rttSlots;
		queuedMessage.direction = Direction::ToStation;
		queuedMessage.stationID = ack->stationID();
		queuedMessage.msg = msg;
		_queue.emplace(queuedMessage.deliveryTime, queuedMessage);
	}
}

void Emulator::enqueueFromStation(
	StationID stationID,
	Timestamp sendTime,
	std::shared_ptr<IMessage> msg)
{
	if (!msg)
		return;

	QueuedMessage queuedMessage;
	queuedMessage.deliveryTime = sendTime + _rttSlots;
	queuedMessage.direction = Direction::ToHub;
	queuedMessage.stationID = stationID;
	queuedMessage.msg = msg;
	_queue.emplace(queuedMessage.deliveryTime, queuedMessage);
}

} // namespace starTopologyEmulator
