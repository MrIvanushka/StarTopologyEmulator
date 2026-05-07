#pragma once

#include <cstddef>
#include <memory>

#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT MetricSinkFactory
{
public:
	struct Config
	{
		std::size_t capacityPerMetric = 0;
	};

	static std::shared_ptr<IMetricSink> make(Config config = {});
};

} // namespace starTopologyEmulator
