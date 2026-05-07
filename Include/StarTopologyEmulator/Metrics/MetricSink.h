#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

using MetricHandle = std::uint32_t;
inline constexpr MetricHandle kInvalidMetricHandle =
	static_cast<MetricHandle>(-1);

struct MetricSample
{
	std::uint64_t frame = 0;
	double value = 0.0;
};

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT IMetricSink
{
public:
	using SampleCallback = std::function<void(MetricHandle, MetricSample)>;
	using SubscriptionId = std::uint64_t;

	virtual ~IMetricSink() = default;

	virtual MetricHandle registerMetric(std::string path) = 0;

	virtual void emit(MetricHandle handle, std::uint64_t frame, double value) = 0;

	virtual std::string_view path(MetricHandle handle) const = 0;

	virtual MetricHandle find(std::string_view path) const = 0;

	virtual std::vector<std::string> paths() const = 0;

	virtual std::vector<MetricSample> series(MetricHandle handle) const = 0;

	virtual std::vector<MetricSample> seriesRange(
		MetricHandle handle,
		std::uint64_t fromFrame,
		std::uint64_t toFrame) const = 0;

	virtual std::vector<MetricSample> lastN(
		MetricHandle handle,
		std::size_t count) const = 0;

	virtual bool latest(MetricHandle handle, MetricSample& out) const = 0;

	virtual SubscriptionId subscribe(SampleCallback callback) = 0;

	virtual void unsubscribe(SubscriptionId id) = 0;
};

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT MetricScope
{
public:
	MetricScope() = default;

	MetricScope(std::shared_ptr<IMetricSink> sink, std::string prefix)
		: _sink(std::move(sink)), _prefix(std::move(prefix))
	{}

	bool active() const { return _sink != nullptr; }

	const std::string& prefix() const { return _prefix; }

	std::shared_ptr<IMetricSink> sink() const { return _sink; }

	MetricScope child(std::string folderName) const
	{
		if (!_sink)
			return {};
		std::string newPrefix = _prefix.empty()
			? std::move(folderName)
			: _prefix + '/' + std::move(folderName);
		return MetricScope(_sink, std::move(newPrefix));
	}

	MetricHandle registerMetric(std::string leafName) const
	{
		if (!_sink)
			return kInvalidMetricHandle;
		std::string fullPath = _prefix.empty()
			? std::move(leafName)
			: _prefix + '/' + std::move(leafName);
		return _sink->registerMetric(std::move(fullPath));
	}

	void emit(MetricHandle handle, std::uint64_t frame, double value) const
	{
		if (!_sink || handle == kInvalidMetricHandle)
			return;
		_sink->emit(handle, frame, value);
	}

private:
	std::shared_ptr<IMetricSink> _sink;
	std::string _prefix;
};

} // namespace starTopologyEmulator
