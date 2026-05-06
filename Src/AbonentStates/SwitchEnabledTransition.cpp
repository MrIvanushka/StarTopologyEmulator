#include "SwitchEnabledTransition.h"

namespace starTopologyEmulator
{

SwitchEnabledTransition::SwitchEnabledTransition(
	std::shared_ptr<StationContext> context,
	IState* nextState,
	bool expectedValue)
	: _context(context)
	, _nextState(nextState)
	, _expectedValue(expectedValue)
{
}

bool SwitchEnabledTransition::needTransit()
{
	return _context->enabled == _expectedValue;
}

IState* SwitchEnabledTransition::nextState()
{
	return _nextState;
}

} // namespace starTopologyEmulator
