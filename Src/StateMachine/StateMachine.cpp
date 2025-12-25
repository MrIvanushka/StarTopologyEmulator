#include "StarTopologyEmulator/StateMachine/StateMachine.h"

namespace starTopologyEmulator
{

StateMachine::StateMachine(const StateCollection& states) :
	_states{ states },
	_currentState{ nullptr }
{
	if (!_states.empty())
		_currentState = _states[0].get();
}

void StateMachine::update()
{
	if (!_currentState)
		return;

	_currentState = _currentState->update();
}

IState::Index StateMachine::currentStateIndex() const
{
	return _currentState ? _currentState->index() : IState::INVALID;
}

} // namespace starTopologyEmulator
