#include "StarTopologyEmulator/BacklogAccumulator/BacklogAccumulatorFactory.h"

#include "BacklogAccumulator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IBacklogAccumulator> BacklogAccumulatorFactory::make(std::uint64_t bitsPerSlot, MetricScope scope)
{
	return std::make_unique<BacklogAccumulator>(bitsPerSlot, std::move(scope));
}

} // namespace starTopologyEmulator
