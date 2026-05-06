#include "StarTopologyEmulator/StateMachine/State.h"

namespace starTopologyEmulator
{

State::State(Index index)
	: _index(index)
{}

void State::setupTransitions(Transitions&& transitions)
{
	_transitions = std::move(transitions);
}

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