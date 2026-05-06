#include <gtest/gtest.h>

#include "StarTopologyEmulator/TrafficProfile/CbrTrafficProfileConfig.h"
#include "StarTopologyEmulator/TrafficProfile/TrafficProfileFactory.h"

using namespace starTopologyEmulator;

TEST(CbrTrafficProfile, ZeroRateProducesZeroBits)
{
	auto profile = TrafficProfileFactory::make(CbrTrafficProfileConfig{});
	EXPECT_EQ(profile->generateBits(1000), 0u);
	EXPECT_EQ(profile->generateBits(1'000'000), 0u);
}

TEST(CbrTrafficProfile, NonPositiveDurationReturnsZero)
{
	CbrTrafficProfileConfig cfg;
	cfg.bitsPerTimestamp = 5.0;
	auto profile = TrafficProfileFactory::make(cfg);
	EXPECT_EQ(profile->generateBits(0), 0u);
	EXPECT_EQ(profile->generateBits(-100), 0u);
}

TEST(CbrTrafficProfile, IntegerRateProducesExactBits)
{
	CbrTrafficProfileConfig cfg;
	cfg.bitsPerTimestamp = 4.0;
	auto profile = TrafficProfileFactory::make(cfg);
	EXPECT_EQ(profile->generateBits(10), 40u);
	EXPECT_EQ(profile->generateBits(100), 400u);
}

TEST(CbrTrafficProfile, FractionalRateAccumulatesResidualBetweenCalls)
{
	CbrTrafficProfileConfig cfg;
	cfg.bitsPerTimestamp = 0.5;
	auto profile = TrafficProfileFactory::make(cfg);

	// Each call generates 0.5 bits. First yields 0, residual=0.5; second yields 1.
	EXPECT_EQ(profile->generateBits(1), 0u);
	EXPECT_EQ(profile->generateBits(1), 1u);
	EXPECT_EQ(profile->generateBits(1), 0u);
	EXPECT_EQ(profile->generateBits(1), 1u);
}

TEST(CbrTrafficProfile, LongIntervalCoversManyBits)
{
	CbrTrafficProfileConfig cfg;
	cfg.bitsPerTimestamp = 1.5;
	auto profile = TrafficProfileFactory::make(cfg);

	// Over 100 ticks: 150 bits exactly.
	EXPECT_EQ(profile->generateBits(100), 150u);
}
