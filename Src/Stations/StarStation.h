#pragma once

#include <functional>
#include <random>

#include "StarTopologyEmulator/IFaces/IStarStation.h"
#include "StarTopologyEmulator/StateMachine/StateMachine.h"
#include "StationContext.h"

namespace starTopologyEmulator
{

class StarStation : public IStarStation
{
public:
	StarStation(
		std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
		StationID id,
		int messagesNeeded,
		int rttSlots,
		std::mt19937& rng);

	void update(Timestamp currentTime) override;

	void handleMessage(std::shared_ptr<IMessage>, Timestamp) override;

	TerminalState currentState() const;

	std::optional<Timestamp> joinedTime() const;

	StationID id() const;

private:
	void buildStateMachine();

	std::shared_ptr<StationContext> _context;
	std::unique_ptr<StateMachine> _stateMachine;
	int _rttSlots;
	std::mt19937& _rng;
};

} // namespace starTopologyEmulator