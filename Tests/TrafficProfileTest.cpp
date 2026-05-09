#include <gtest/gtest.h>

#include "StarTopologyEmulator/TrafficProfile/BurstTrafficProfileConfig.h"
#include "StarTopologyEmulator/TrafficProfile/CbrTrafficProfileConfig.h"
#include "StarTopologyEmulator/TrafficProfile/PoissonTrafficProfileConfig.h"
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

TEST(PoissonTrafficProfile, ZeroRateProducesZeroBits)
{
	PoissonTrafficProfileConfig cfg;
	cfg.bitsPerPacket = 100;
	cfg.seed = 42;
	auto profile = TrafficProfileFactory::make(cfg);
	EXPECT_EQ(profile->generateBits(1000), 0u);
	EXPECT_EQ(profile->generateBits(1'000'000), 0u);
}

TEST(PoissonTrafficProfile, ZeroPacketSizeProducesZeroBits)
{
	PoissonTrafficProfileConfig cfg;
	cfg.packetsPerTimestamp = 0.5;
	cfg.seed = 42;
	auto profile = TrafficProfileFactory::make(cfg);
	EXPECT_EQ(profile->generateBits(1000), 0u);
}

TEST(PoissonTrafficProfile, NonPositiveDurationReturnsZero)
{
	PoissonTrafficProfileConfig cfg;
	cfg.packetsPerTimestamp = 0.1;
	cfg.bitsPerPacket = 100;
	cfg.seed = 42;
	auto profile = TrafficProfileFactory::make(cfg);
	EXPECT_EQ(profile->generateBits(0), 0u);
	EXPECT_EQ(profile->generateBits(-100), 0u);
}

TEST(PoissonTrafficProfile, GeneratesMultiplesOfPacketSize)
{
	PoissonTrafficProfileConfig cfg;
	cfg.packetsPerTimestamp = 0.05;
	cfg.bitsPerPacket = 80;
	cfg.seed = 12345;
	auto profile = TrafficProfileFactory::make(cfg);

	for (int i = 0; i < 100; ++i)
	{
		const std::uint64_t bits = profile->generateBits(100);
		EXPECT_EQ(bits % cfg.bitsPerPacket, 0u);
	}
}

TEST(PoissonTrafficProfile, SameSeedProducesSameSequence)
{
	PoissonTrafficProfileConfig cfg;
	cfg.packetsPerTimestamp = 0.1;
	cfg.bitsPerPacket = 100;
	cfg.seed = 7;

	auto a = TrafficProfileFactory::make(cfg);
	auto b = TrafficProfileFactory::make(cfg);

	for (int i = 0; i < 50; ++i)
		EXPECT_EQ(a->generateBits(50), b->generateBits(50));
}

TEST(PoissonTrafficProfile, EmpiricalMeanApproachesLambdaTimesPacketBits)
{
	PoissonTrafficProfileConfig cfg;
	cfg.packetsPerTimestamp = 0.1;
	cfg.bitsPerPacket = 100;
	cfg.seed = 2024;
	auto profile = TrafficProfileFactory::make(cfg);

	const Timestamp duration = 100;
	const std::uint64_t expectedPerCall = 1000;
	const int iterations = 5000;

	std::uint64_t total = 0;
	for (int i = 0; i < iterations; ++i)
		total += profile->generateBits(duration);

	const double meanBits = static_cast<double>(total) / iterations;
	EXPECT_NEAR(meanBits, static_cast<double>(expectedPerCall), expectedPerCall * 0.05);
}

TEST(BurstTrafficProfile, ZeroOnRateProducesZeroBits)
{
	BurstTrafficProfileConfig cfg;
	cfg.bitsPerPacket = 100;
	cfg.meanOnDuration = 50.0;
	cfg.meanOffDuration = 50.0;
	cfg.seed = 42;
	auto profile = TrafficProfileFactory::make(cfg);
	EXPECT_EQ(profile->generateBits(10000), 0u);
}

TEST(BurstTrafficProfile, ZeroPhaseDurationProducesZeroBits)
{
	BurstTrafficProfileConfig cfg;
	cfg.packetsPerTimestampOn = 0.5;
	cfg.bitsPerPacket = 100;
	cfg.meanOffDuration = 50.0;
	cfg.seed = 42;
	auto profile = TrafficProfileFactory::make(cfg);
	EXPECT_EQ(profile->generateBits(1000), 0u);
}

TEST(BurstTrafficProfile, NonPositiveDurationReturnsZero)
{
	BurstTrafficProfileConfig cfg;
	cfg.packetsPerTimestampOn = 0.5;
	cfg.bitsPerPacket = 100;
	cfg.meanOnDuration = 50.0;
	cfg.meanOffDuration = 50.0;
	cfg.seed = 42;
	auto profile = TrafficProfileFactory::make(cfg);
	EXPECT_EQ(profile->generateBits(0), 0u);
	EXPECT_EQ(profile->generateBits(-100), 0u);
}

TEST(BurstTrafficProfile, GeneratesMultiplesOfPacketSize)
{
	BurstTrafficProfileConfig cfg;
	cfg.packetsPerTimestampOn = 0.2;
	cfg.bitsPerPacket = 64;
	cfg.meanOnDuration = 30.0;
	cfg.meanOffDuration = 70.0;
	cfg.seed = 12345;
	auto profile = TrafficProfileFactory::make(cfg);

	for (int i = 0; i < 200; ++i)
	{
		const std::uint64_t bits = profile->generateBits(50);
		EXPECT_EQ(bits % cfg.bitsPerPacket, 0u);
	}
}

TEST(BurstTrafficProfile, SameSeedProducesSameSequence)
{
	BurstTrafficProfileConfig cfg;
	cfg.packetsPerTimestampOn = 0.5;
	cfg.bitsPerPacket = 100;
	cfg.meanOnDuration = 20.0;
	cfg.meanOffDuration = 80.0;
	cfg.seed = 7;

	auto a = TrafficProfileFactory::make(cfg);
	auto b = TrafficProfileFactory::make(cfg);

	for (int i = 0; i < 50; ++i)
		EXPECT_EQ(a->generateBits(50), b->generateBits(50));
}

TEST(BurstTrafficProfile, EmpiricalMeanMatchesFormula2)
{
	BurstTrafficProfileConfig cfg;
	cfg.packetsPerTimestampOn = 1.0;
	cfg.bitsPerPacket = 10;
	cfg.meanOnDuration = 30.0;
	cfg.meanOffDuration = 70.0;
	cfg.seed = 2024;
	auto profile = TrafficProfileFactory::make(cfg);

	const Timestamp duration = 1000;
	const int iterations = 200;

	std::uint64_t total = 0;
	for (int i = 0; i < iterations; ++i)
		total += profile->generateBits(duration);

	const double avgRate = cfg.packetsPerTimestampOn
		* cfg.meanOnDuration / (cfg.meanOnDuration + cfg.meanOffDuration);
	const double expectedTotalBits = avgRate
		* static_cast<double>(duration) * iterations
		* static_cast<double>(cfg.bitsPerPacket);

	EXPECT_NEAR(static_cast<double>(total), expectedTotalBits, expectedTotalBits * 0.05);
}
