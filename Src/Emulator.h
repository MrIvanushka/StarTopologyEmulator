#pragma once

#include <functional>
#include <map>
#include <memory>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

class Emulator : public IEmulator
{
	DECLARE_METRICS("Эмулятор СЛС")
public:
	using SendFunc = std::function<void(Timestamp, std::shared_ptr<IMessage>)>;

	Emulator(
		std::function<std::shared_ptr<IStarStation>(SendFunc, StationID)>,
		std::function<std::shared_ptr<IStarHub>(SendFunc)>,
		int stationCount);

	void update(Timestamp currentTime) override;

	std::shared_ptr<IStarHub> hub() const override;

	const std::vector<std::shared_ptr<IStarStation>>& stations() const override;
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

        std::uint32_t joinedStationsCount() const;
private:
	std::unique_ptr<IFrameCalculator> _frameCalculator;

	std::shared_ptr<IStarHub> _hub;
	std::vector<std::shared_ptr<IStarStation>> _stations;

	std::multimap<Timestamp, QueuedMessage> _queue;
};

} // namespace starTopologyEmulator

