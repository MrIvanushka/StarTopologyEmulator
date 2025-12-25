#include "StarTopologyEmulator/StateMachine/State.h"

namespace starTopologyEmulator
{

State::State(const Transitions& transitions, Index index)
	: _transitions(transitions)
	, _index(index)
{}

State::NextStatePtr State::update()
{
	onUpdate();

	for (auto transition : _transitions)
	{
		if (transition->needTransit())
			return transition->nextState();
	}
	return this;
}

} // namespace starTopologyEmulator