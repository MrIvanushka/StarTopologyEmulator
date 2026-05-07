#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/Metrics/MetricSinkFactory.h"

namespace starTopologyEmulator
{

class TimeSeriesMetricSink : public IMetricSink
{
public:
	using Config = MetricSinkFactory::Config;

	explicit TimeSeriesMetricSink(Config config = {});

	MetricHandle registerMetric(std::string path) override;

	void emit(MetricHandle handle, std::uint64_t frame, double value) override;

	std::string_view path(MetricHandle handle) const override;

	MetricHandle find(std::string_view path) const override;

	std::vector<std::string> paths() const override;

	std::vector<MetricSample> series(MetricHandle handle) const override;

	std::vector<MetricSample> seriesRange(
		MetricHandle handle,
		std::uint64_t fromFrame,
		std::uint64_t toFrame) const override;

	std::vector<MetricSample> lastN(
		MetricHandle handle,
		std::size_t count) const override;

	bool latest(MetricHandle handle, MetricSample& out) const override;

	SubscriptionId subscribe(SampleCallback callback) override;

	void unsubscribe(SubscriptionId id) override;

private:
	struct Series
	{
		std::vector<MetricSample> samples;
	};

	void appendSample(Series& s, MetricSample sample);

	const Config _config;

	mutable std::shared_mutex _mutex;
	std::vector<std::string> _paths;
	std::unordered_map<std::string, MetricHandle> _byPath;
	std::vector<Series> _series;

	mutable std::mutex _subscribersMutex;
	SubscriptionId _nextSubscriptionId = 1;
	std::vector<std::pair<SubscriptionId, SampleCallback>> _subscribers;
};

} // namespace starTopologyEmulator
