#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/StateMachine/State.h"
#include "Stations/StationContext.h"

namespace starTopologyEmulator
{

class OperationState final : public State
{
public:
	OperationState(std::shared_ptr<StationContext>);

private:
	void onUpdate() override;

	std::shared_ptr<StationContext> _context;

	std::uint64_t _lastProcessedFrame = 0;
};

} // namespace starTopologyEmulator
