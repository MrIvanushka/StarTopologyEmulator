#include "OffState.h"

#include "StarTopologyEmulator/TerminalState.h"

namespace starTopologyEmulator
{

OffState::OffState(const Transitions& transitions)
	: State(transitions, static_cast<Index>(TerminalState::OFF))
{}


} // namespace starTopologyEmulator
