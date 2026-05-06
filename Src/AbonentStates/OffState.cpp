#include "OffState.h"

#include "StarTopologyEmulator/TerminalState.h"

namespace starTopologyEmulator
{

OffState::OffState()
	: State(static_cast<Index>(TerminalState::OFF))
{}


} // namespace starTopologyEmulator
