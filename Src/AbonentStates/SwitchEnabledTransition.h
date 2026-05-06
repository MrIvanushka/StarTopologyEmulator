#pragma once

#include <memory>

#include "StarTopologyEmulator/StateMachine/IFaces/ITransition.h"
#include "Stations/StationContext.h"

namespace starTopologyEmulator
{

class SwitchEnabledTransition : public ITransition
{
public:
	SwitchEnabledTransition(
		std::shared_ptr<StationContext> context,
		IState* nextState,
		bool expectedValue);

	bool needTransit() override;

	IState* nextState() override;

private:
    std::shared_ptr<StationContext> _context;
    IState* _nextState;
    bool _expectedValue;
};


} // namespace starTopologyEmulator
