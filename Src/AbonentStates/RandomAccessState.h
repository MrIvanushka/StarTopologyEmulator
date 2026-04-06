#pragma once

#include <memory>

#include "StarTopologyEmulator/StateMachine/State.h"
#include "Stations/StationContext.h"

namespace starTopologyEmulator
{

class RandomAccessState final : public State
{
public:
	RandomAccessState(
		std::shared_ptr<StationContext>,
		const Transitions&);

private:
	void onUpdate() override;

	void handleCollision(std::shared_ptr<StarHubPlanMessage>);

	void implementBackoffLogic(std::shared_ptr<StarHubPlanMessage>);

	void transmit();

	std::shared_ptr<StationContext> _context;

	std::uint64_t _lastProcessedFrame = 0;
	std::uint64_t _lastProcessedSlot = 0;
};

} // namespace starTopologyEmulator
