#pragma once

#include <functional>
#include <map>
#include <memory>

#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

class Emulator : public IEmulator
{
public:
	using SendFunc = std::function<void(Timestamp, std::shared_ptr<IMessage>)>;
	using RTT = int;

	Emulator(
		std::function<std::shared_ptr<IStarStation>(SendFunc, StationID, RTT)>,
		std::function<std::shared_ptr<IStarHub>(SendFunc, const std::vector<StationID>&)>,
		int stationCount, int rttSlots);

	void update(Timestamp currentTime) override;

	std::shared_ptr<IStarHub> hub() const override;

	const std::vector<std::shared_ptr<IStarStation>>& stations() const override;

	const std::map<StationID, StateCounter>& stationStats() const override;

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

	int _rttSlots;

	std::shared_ptr<IStarHub> _hub;

	std::vector<std::shared_ptr<IStarStation>> _stations;

	std::multimap<Timestamp, QueuedMessage> _queue;

	std::map<StationID, StateCounter> _stationStats;
};

} // namespace starTopologyEmulator
