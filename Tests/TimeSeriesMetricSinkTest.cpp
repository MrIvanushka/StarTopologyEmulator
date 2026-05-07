#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/Metrics/MetricSinkFactory.h"

using namespace starTopologyEmulator;

TEST(MetricSink, RegisterReturnsStableHandleForSamePath)
{
	auto sink = MetricSinkFactory::make();
	const auto h1 = sink->registerMetric("a/b");
	const auto h2 = sink->registerMetric("a/b");
	EXPECT_EQ(h1, h2);
	EXPECT_NE(h1, kInvalidMetricHandle);
}

TEST(MetricSink, FindByPathReturnsRegisteredHandle)
{
	auto sink = MetricSinkFactory::make();
	const auto h = sink->registerMetric("foo/bar");
	EXPECT_EQ(sink->find("foo/bar"), h);
	EXPECT_EQ(sink->find("missing"), kInvalidMetricHandle);
}

TEST(MetricSink, EmittedSamplesAreReadable)
{
	auto sink = MetricSinkFactory::make();
	const auto h = sink->registerMetric("g");

	sink->emit(h, 0, 1.0);
	sink->emit(h, 1, 2.0);
	sink->emit(h, 2, 3.0);

	const auto series = sink->series(h);
	ASSERT_EQ(series.size(), 3u);
	EXPECT_EQ(series[0].frame, 0u);
	EXPECT_DOUBLE_EQ(series[0].value, 1.0);
	EXPECT_EQ(series[2].frame, 2u);
	EXPECT_DOUBLE_EQ(series[2].value, 3.0);
}

TEST(MetricSink, DuplicateFrameOverwritesValue)
{
	auto sink = MetricSinkFactory::make();
	const auto h = sink->registerMetric("g");

	sink->emit(h, 5, 1.0);
	sink->emit(h, 5, 9.0);

	const auto series = sink->series(h);
	ASSERT_EQ(series.size(), 1u);
	EXPECT_EQ(series[0].frame, 5u);
	EXPECT_DOUBLE_EQ(series[0].value, 9.0);
}

TEST(MetricSink, OutOfOrderEmissionIsInsertedSorted)
{
	auto sink = MetricSinkFactory::make();
	const auto h = sink->registerMetric("g");

	sink->emit(h, 3, 0.3);
	sink->emit(h, 1, 0.1);
	sink->emit(h, 2, 0.2);

	const auto series = sink->series(h);
	ASSERT_EQ(series.size(), 3u);
	EXPECT_EQ(series[0].frame, 1u);
	EXPECT_EQ(series[1].frame, 2u);
	EXPECT_EQ(series[2].frame, 3u);
}

TEST(MetricSink, SeriesRangeReturnsInclusiveSlice)
{
	auto sink = MetricSinkFactory::make();
	const auto h = sink->registerMetric("g");

	for (std::uint64_t f = 0; f < 10; ++f)
		sink->emit(h, f, static_cast<double>(f));

	const auto slice = sink->seriesRange(h, 3, 6);
	ASSERT_EQ(slice.size(), 4u);
	EXPECT_EQ(slice.front().frame, 3u);
	EXPECT_EQ(slice.back().frame, 6u);
}

TEST(MetricSink, LastNReturnsTrailingSlice)
{
	auto sink = MetricSinkFactory::make();
	const auto h = sink->registerMetric("g");

	for (std::uint64_t f = 0; f < 5; ++f)
		sink->emit(h, f, static_cast<double>(f));

	const auto last3 = sink->lastN(h, 3);
	ASSERT_EQ(last3.size(), 3u);
	EXPECT_EQ(last3[0].frame, 2u);
	EXPECT_EQ(last3[2].frame, 4u);

	const auto last100 = sink->lastN(h, 100);
	EXPECT_EQ(last100.size(), 5u);
}

TEST(MetricSink, LatestExposesMostRecentSample)
{
	auto sink = MetricSinkFactory::make();
	const auto h = sink->registerMetric("g");

	MetricSample s{};
	EXPECT_FALSE(sink->latest(h, s));

	sink->emit(h, 7, 42.0);
	ASSERT_TRUE(sink->latest(h, s));
	EXPECT_EQ(s.frame, 7u);
	EXPECT_DOUBLE_EQ(s.value, 42.0);
}

TEST(MetricSink, RingBufferDropsOldestWhenCapacityReached)
{
	MetricSinkFactory::Config cfg;
	cfg.capacityPerMetric = 3;
	auto sink = MetricSinkFactory::make(cfg);
	const auto h = sink->registerMetric("g");

	for (std::uint64_t f = 0; f < 5; ++f)
		sink->emit(h, f, static_cast<double>(f));

	const auto series = sink->series(h);
	ASSERT_EQ(series.size(), 3u);
	EXPECT_EQ(series.front().frame, 2u);
	EXPECT_EQ(series.back().frame, 4u);
}

TEST(MetricSink, SubscribersGetCallbackPerEmit)
{
	auto sink = MetricSinkFactory::make();
	const auto h = sink->registerMetric("g");

	std::vector<MetricSample> received;
	const auto sub = sink->subscribe([&](MetricHandle, MetricSample s) {
		received.push_back(s);
	});

	sink->emit(h, 0, 1.0);
	sink->emit(h, 1, 2.0);

	sink->unsubscribe(sub);

	sink->emit(h, 2, 3.0);

	ASSERT_EQ(received.size(), 2u);
	EXPECT_EQ(received[0].frame, 0u);
	EXPECT_EQ(received[1].frame, 1u);
}

TEST(MetricSink, ScopeBuildsHierarchicalPaths)
{
	auto sink = MetricSinkFactory::make();
	MetricScope root(sink, "Root");
	auto child = root.child("Child");
	const auto h = child.registerMetric("Leaf");

	EXPECT_EQ(sink->path(h), "Root/Child/Leaf");

	child.emit(h, 1, 100.0);

	MetricSample latest{};
	ASSERT_TRUE(sink->latest(h, latest));
	EXPECT_DOUBLE_EQ(latest.value, 100.0);
}

TEST(MetricSink, InactiveScopeIsNoop)
{
	MetricScope inactive;
	EXPECT_FALSE(inactive.active());

	const auto h = inactive.registerMetric("never");
	EXPECT_EQ(h, kInvalidMetricHandle);

	inactive.emit(h, 1, 1.0);

	auto child = inactive.child("Sub");
	EXPECT_FALSE(child.active());
}
