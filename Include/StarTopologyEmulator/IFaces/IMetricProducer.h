#pragma once

#include <vector>

#include "StarTopologyEmulator/Metrics/MetricValue.h"

namespace starTopologyEmulator
{

class IMetricProducer
{
public:
	virtual ~IMetricProducer() = default;

	virtual MetricValueFolder collectMetrics() const { return {}; }
};

} // namespace starTopologyEmulator

