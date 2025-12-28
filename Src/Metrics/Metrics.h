#pragma once

#include "MetricFormer.h"

namespace starTopologyEmulator
{

#define DECLARE_METRICS(name) \
private: \
	std::string _metricFolderName = name; \
	std::vector<IMetricProducer*> _metricSubproducers; \
	std::vector<MetricFormer> _metricFormers; \
public: \
	MetricValueFolder collectMetrics() const override { \
		std::vector<MetricValue> values; \
		values.reserve(_metricFormers.size()); \
		for (const auto& former : _metricFormers) \
			values.push_back(former.makeValue()); \
		std::vector<MetricValueFolder> subFolders; \
		subFolders.reserve(_metricSubproducers.size()); \
		for (const auto& subProducer : _metricSubproducers) \
			subFolders.push_back(subProducer->collectMetrics()); \
		return MetricValueFolder(_metricFolderName, std::move(subFolders), std::move(values)); \
	} \
private:

#define REGISTER_METRIC(field, label) \
	_metricFormers.emplace_back(label, [this]() { return static_cast<double>(field); })

#define REGISTER_METRIC_SUBFOLDER(subFolder) \
	_metricSubproducers.push_back(subFolder)

} // namespace starTopologyEmulator