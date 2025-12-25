#include "OperationState.h"

#include "StarTopologyEmulator/TerminalState.h"

namespace starTopologyEmulator
{

OperationState::OperationState(const Transitions& transitions)
	: State(transitions, static_cast<Index>(TerminalState::OPERATION))
{
}


} // namespace starTopologyEmulator
