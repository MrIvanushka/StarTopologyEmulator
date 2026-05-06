#include "StarTopologyEmulator/BacklogAccumulator/BacklogAccumulatorFactory.h"

#include "BacklogAccumulator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IBacklogAccumulator> BacklogAccumulatorFactory::make(std::uint64_t bitsPerSlot)
{
	return std::make_unique<BacklogAccumulator>(bitsPerSlot);
}

} // namespace starTopologyEmulator
