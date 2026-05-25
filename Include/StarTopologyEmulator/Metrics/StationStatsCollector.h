#pragma once

#include <cstdint>
#include <optional>

#include "StarTopologyEmulator/IFaces/IStationStatsCollector.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StationStatsCollector : public IStationStatsCollector
{
public:
	explicit StationStatsCollector(MetricScope scope = {});

	void onPtxCheck(std::uint64_t frame) override;

private:
	void rollOverIfNeeded(std::uint64_t frame);

	MetricScope _scope;
	MetricHandle _hPtxCheckCount = kInvalidMetricHandle;
	std::optional<std::uint64_t> _currentFrame;
	std::uint64_t _ptxCheckCount = 0;
};

} // namespace starTopologyEmulator
