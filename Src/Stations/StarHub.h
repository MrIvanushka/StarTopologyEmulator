#pragma once

#include <functional>
#include <map>

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/Messages/StarStationMessage.h"
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"

namespace starTopologyEmulator
{

class StarHub : public IStarHub
{
public:
	StarHub(
		std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
		const std::vector<StationID>& terminalIDs,
		std::unique_ptr<IStarHubStrategy> strategy);

	void update(Timestamp currentTime) override;

	void handleMessage(std::shared_ptr<IMessage>, Timestamp) override;

	double incomeLoad() const override;

	double incomePlr() const override;

	std::uint64_t getTotalSuccess() const override;

	std::uint64_t getTotalCollisions() const override;

private:
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> _sendFunc;
	std::vector<StationID> _terminalIDs;
	std::unique_ptr<IStarHubStrategy> _strategy;

	std::map<Timestamp, std::vector<std::shared_ptr<IMessage>>> _pending;

	std::uint64_t _totalSlots;
	std::uint64_t _totalAttempts;
	std::uint64_t _totalLost;
	std::uint64_t _totalSuccess;
	std::uint64_t _totalCollisions;
};

} // namespace starTopologyEmulator

