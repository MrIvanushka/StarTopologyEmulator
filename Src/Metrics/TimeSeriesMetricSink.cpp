#include "TimeSeriesMetricSink.h"

#include <algorithm>
#include <utility>

namespace starTopologyEmulator
{

TimeSeriesMetricSink::TimeSeriesMetricSink(Config config)
	: _config(config)
{}

MetricHandle TimeSeriesMetricSink::registerMetric(std::string path)
{
	std::unique_lock lock(_mutex);

	if (auto it = _byPath.find(path); it != _byPath.end())
		return it->second;

	const auto handle = static_cast<MetricHandle>(_paths.size());
	_paths.push_back(path);
	_series.emplace_back();
	_byPath.emplace(std::move(path), handle);
	return handle;
}

void TimeSeriesMetricSink::appendSample(Series& s, MetricSample sample)
{
	if (!s.samples.empty() && s.samples.back().frame == sample.frame)
	{
		s.samples.back().value = sample.value;
		return;
	}

	if (!s.samples.empty() && sample.frame < s.samples.back().frame)
	{
		auto it = std::lower_bound(
			s.samples.begin(),
			s.samples.end(),
			sample.frame,
			[](const MetricSample& lhs, std::uint64_t f) { return lhs.frame < f; });

		if (it != s.samples.end() && it->frame == sample.frame)
			it->value = sample.value;
		else
			s.samples.insert(it, sample);

		return;
	}

	s.samples.push_back(sample);

	if (_config.capacityPerMetric > 0
		&& s.samples.size() > _config.capacityPerMetric)
	{
		const std::size_t excess = s.samples.size() - _config.capacityPerMetric;
		s.samples.erase(s.samples.begin(), s.samples.begin() + excess);
	}
}

void TimeSeriesMetricSink::emit(MetricHandle handle, std::uint64_t frame, double value)
{
	{
		std::unique_lock lock(_mutex);
		if (handle == kInvalidMetricHandle || handle >= _series.size())
			return;
		appendSample(_series[handle], MetricSample{ frame, value });
	}

	std::vector<SampleCallback> callbacks;
	{
		std::lock_guard lock(_subscribersMutex);
		callbacks.reserve(_subscribers.size());
		for (const auto& [id, cb] : _subscribers)
			callbacks.push_back(cb);
	}

	const MetricSample sample{ frame, value };
	for (auto& cb : callbacks)
		cb(handle, sample);
}

std::string_view TimeSeriesMetricSink::path(MetricHandle handle) const
{
	std::shared_lock lock(_mutex);
	if (handle >= _paths.size())
		return {};
	return _paths[handle];
}

MetricHandle TimeSeriesMetricSink::find(std::string_view path) const
{
	std::shared_lock lock(_mutex);
	const auto it = _byPath.find(std::string(path));
	if (it == _byPath.end())
		return kInvalidMetricHandle;
	return it->second;
}

std::vector<std::string> TimeSeriesMetricSink::paths() const
{
	std::shared_lock lock(_mutex);
	return _paths;
}

std::vector<MetricSample> TimeSeriesMetricSink::series(MetricHandle handle) const
{
	std::shared_lock lock(_mutex);
	if (handle >= _series.size())
		return {};
	return _series[handle].samples;
}

std::vector<MetricSample> TimeSeriesMetricSink::seriesRange(
	MetricHandle handle,
	std::uint64_t fromFrame,
	std::uint64_t toFrame) const
{
	std::shared_lock lock(_mutex);
	if (handle >= _series.size())
		return {};

	const auto& samples = _series[handle].samples;

	const auto begin = std::lower_bound(
		samples.begin(),
		samples.end(),
		fromFrame,
		[](const MetricSample& s, std::uint64_t f) { return s.frame < f; });

	const auto end = std::upper_bound(
		samples.begin(),
		samples.end(),
		toFrame,
		[](std::uint64_t f, const MetricSample& s) { return f < s.frame; });

	return std::vector<MetricSample>(begin, end);
}

std::vector<MetricSample> TimeSeriesMetricSink::lastN(
	MetricHandle handle,
	std::size_t count) const
{
	std::shared_lock lock(_mutex);
	if (handle >= _series.size())
		return {};

	const auto& samples = _series[handle].samples;
	if (count >= samples.size())
		return samples;

	return std::vector<MetricSample>(samples.end() - count, samples.end());
}

bool TimeSeriesMetricSink::latest(MetricHandle handle, MetricSample& out) const
{
	std::shared_lock lock(_mutex);
	if (handle >= _series.size())
		return false;

	const auto& samples = _series[handle].samples;
	if (samples.empty())
		return false;

	out = samples.back();
	return true;
}

IMetricSink::SubscriptionId TimeSeriesMetricSink::subscribe(SampleCallback callback)
{
	std::lock_guard lock(_subscribersMutex);
	const auto id = _nextSubscriptionId++;
	_subscribers.emplace_back(id, std::move(callback));
	return id;
}

void TimeSeriesMetricSink::unsubscribe(SubscriptionId id)
{
	std::lock_guard lock(_subscribersMutex);
	_subscribers.erase(
		std::remove_if(
			_subscribers.begin(),
			_subscribers.end(),
			[id](const auto& kv) { return kv.first == id; }),
		_subscribers.end());
}

} // namespace starTopologyEmulator
