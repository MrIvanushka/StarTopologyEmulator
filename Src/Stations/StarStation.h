#pragma once

#include <functional>
#include <random>

#include "StarTopologyEmulator/IFaces/IStarStation.h"
#include "StarTopologyEmulator/IFaces/IStationStatsCollector.h"
#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/StateMachine/StateMachine.h"
#include "StationContext.h"

namespace starTopologyEmulator
{

class StarStation : public IStarStation
{
public:
	StarStation(
		std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
		std::unique_ptr<IFrameCalculator> frameCalculator,
		std::unique_ptr<IDynamicFrameSettings> dynamicFrameSettings,
		std::unique_ptr<ITrafficProfile> trafficProfile,
		StationID id,
		int messagesNeeded,
		Timestamp tts,
		std::mt19937& rng,
		std::shared_ptr<IStationStatsCollector> stats = nullptr);

	void update(Timestamp currentTime) override;

	void handleMessage(std::shared_ptr<IMessage>, Timestamp) override;

	void setEnabled(bool) override;

	void setTrafficProfile(std::unique_ptr<ITrafficProfile>) override;

	Timestamp tts() const override;

	TerminalState currentState() const override;

	std::optional<Timestamp> joinedTime() const override;

	StationID id() const override;
private:
	void buildStateMachine();

	std::shared_ptr<StationContext> _context;
	std::unique_ptr<StateMachine> _stateMachine;
	Timestamp _tts;
	std::mt19937& _rng;
};

} // namespace starTopologyEmulator