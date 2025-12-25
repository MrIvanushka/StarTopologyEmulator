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

	int computeWindow() const;

	std::shared_ptr<StationContext> _context;
};

} // namespace starTopologyEmulator
