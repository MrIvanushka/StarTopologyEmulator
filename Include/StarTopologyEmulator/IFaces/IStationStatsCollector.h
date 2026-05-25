#pragma once

#include <cstdint>

namespace starTopologyEmulator
{

class IStationStatsCollector
{
public:
	virtual ~IStationStatsCollector() = default;

	virtual void onPtxCheck(std::uint64_t frame) = 0;
};

} // namespace starTopologyEmulator
