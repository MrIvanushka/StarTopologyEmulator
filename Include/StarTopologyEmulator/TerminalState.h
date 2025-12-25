#pragma once

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

enum class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT TerminalState : int
{
	OFF = 0,
	ACQUISITION,
	AUTH,
	OPERATION
};

} // namespace starTopologyEmulator
