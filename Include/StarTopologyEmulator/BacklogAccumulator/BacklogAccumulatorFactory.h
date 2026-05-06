#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT BacklogAccumulatorFactory
{
public:
	static std::unique_ptr<IBacklogAccumulator> make(std::uint64_t bitsPerSlot);
};

} // namespace starTopologyEmulator
