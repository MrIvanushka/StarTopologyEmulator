#include "StarTopologyEmulator/Metrics/MetricSinkFactory.h"

#include "Metrics/TimeSeriesMetricSink.h"

namespace starTopologyEmulator
{

std::shared_ptr<IMetricSink> MetricSinkFactory::make(Config config)
{
	return std::make_shared<TimeSeriesMetricSink>(config);
}

} // namespace starTopologyEmulator
