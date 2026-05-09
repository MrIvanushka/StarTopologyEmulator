#include <gtest/gtest.h>

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
	// Pois(λ·Δt) over many calls: mean bits per call → λ · Δt · bitsPerPacket.
	PoissonTrafficProfileConfig cfg;
	cfg.packetsPerTimestamp = 0.1;
	cfg.bitsPerPacket = 100;
	cfg.seed = 2024;
	auto profile = TrafficProfileFactory::make(cfg);

	const Timestamp duration = 100;       // expected packets per call: 10
	const std::uint64_t expectedPerCall = 1000; // 10 packets * 100 bits
	const int iterations = 5000;

	std::uint64_t total = 0;
	for (int i = 0; i < iterations; ++i)
		total += profile->generateBits(duration);

	const double meanBits = static_cast<double>(total) / iterations;
	// Std error of mean ≈ sqrt(λΔt)/N · bitsPerPacket ≈ 4.47 bits.
	// 5% slack covers >10σ deviations.
	EXPECT_NEAR(meanBits, static_cast<double>(expectedPerCall), expectedPerCall * 0.05);
}
