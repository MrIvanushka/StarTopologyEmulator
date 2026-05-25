#pragma once

#include <memory>

#include "StarTopologyEmulator/StateMachine/State.h"
#include "Stations/StationContext.h"

namespace starTopologyEmulator
{

class RandomAccessState final : public State
{
public:
	RandomAccessState(std::shared_ptr<StationContext>);

private:
	void onUpdate() override;

	void handleCollision(std::shared_ptr<StarHubPlanMessage>);

	void handleObservedCollision(std::shared_ptr<StarHubPlanMessage>);

	void handleSuccess(std::shared_ptr<StarHubPlanMessage>);

	void planFrameAttempt(std::shared_ptr<StarHubPlanMessage>);

	void transmit();

	std::shared_ptr<StationContext> _context;

	std::uint64_t _lastProcessedFrame = 0;
	std::uint64_t _lastProcessedSlot = 0;

	std::uint64_t _decisionFrame = 0;
	bool _decisionMade = false;
	int _chosenSlot = -1;

	int _deliveredSnapshot = 0;

	std::uint64_t _lastObservedPlanFrame = 0;
	bool _ownCollisionSinceLastPlan = false;
};

} // namespace starTopologyEmulator
