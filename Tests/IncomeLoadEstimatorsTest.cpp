#include <gtest/gtest.h>

#include "Helpers.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/IncomeLoadEstimatorFactory.h"

using namespace starTopologyEmulator;

namespace
{

RandomAccessFrameResult makeFrame(
	std::uint32_t total,
	std::uint32_t idle,
	std::uint32_t success,
	std::uint32_t collision)
{
	RandomAccessFrameResult r{};
	r.totalRaSlots = total;
	r.idleSlots = idle;
	r.successSlots = success;
	r.collisionSlots = collision;
	return r;
}

}

TEST(EmaIncomeLoadEstimator, InitialStateIsZero)
{
	auto est = IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig{});
	EXPECT_DOUBLE_EQ(est->incomeLoad(), 0.0);
	EXPECT_DOUBLE_EQ(est->plr(), 0.0);
}

TEST(EmaIncomeLoadEstimator, ZeroTotalSlotsKeepsState)
{
	auto est = IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig{});
	est->update(makeFrame(0, 0, 0, 0));
	EXPECT_DOUBLE_EQ(est->incomeLoad(), 0.0);
	EXPECT_DOUBLE_EQ(est->plr(), 0.0);
}

TEST(EmaIncomeLoadEstimator, AllIdleProducesLowLoad)
{
	auto est = IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig{});
	est->update(makeFrame(/*total=*/10, /*idle=*/10, /*succ=*/0, /*coll=*/0));
	// Pure idle => G estimate at minimum (close to zero / epsilon).
	EXPECT_LT(est->incomeLoad(), 0.1);
	EXPECT_DOUBLE_EQ(est->plr(), 0.0);
}

TEST(EmaIncomeLoadEstimator, CollisionsRaisePlrToOne)
{
	auto est = IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig{});
	est->update(makeFrame(10, /*idle=*/0, /*succ=*/0, /*coll=*/10));
	EXPECT_DOUBLE_EQ(est->plr(), 1.0);
	EXPECT_GT(est->incomeLoad(), 0.0);
}

TEST(EmaIncomeLoadEstimator, AlphaOneReproducesInstant)
{
	EmaIncomeLoadEstimatorConfig cfg;
	cfg.alphaG = 1.0;
	cfg.alphaPlr = 1.0;
	auto est = IncomeLoadEstimatorFactory::make(cfg);

	est->update(makeFrame(10, 5, 5, 0));
	const double g1 = est->incomeLoad();

	est->update(makeFrame(10, 9, 1, 0));
	// With alpha=1 and isFirstUpdate consumed, the second update follows instant.
	// The two updates must produce different smoothedG since instant changed.
	EXPECT_NE(est->incomeLoad(), g1);
}

TEST(EmaIncomeLoadEstimator, ResetReturnsToZero)
{
	auto est = IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig{});
	est->update(makeFrame(10, 0, 5, 5));
	EXPECT_GT(est->incomeLoad(), 0.0);

	est->reset();
	EXPECT_DOUBLE_EQ(est->incomeLoad(), 0.0);
	EXPECT_DOUBLE_EQ(est->plr(), 0.0);
}

TEST(EmaIncomeLoadEstimator, PlrAlwaysInUnitInterval)
{
	auto est = IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig{});
	for (int i = 0; i < 50; ++i)
	{
		est->update(makeFrame(10, 1, 4, 5));
		EXPECT_GE(est->plr(), 0.0);
		EXPECT_LE(est->plr(), 1.0);
	}
}

TEST(KalmanIncomeLoadEstimator, InitialStateIsZero)
{
	auto est = IncomeLoadEstimatorFactory::make(KalmanIncomeLoadEstimatorConfig{});
	EXPECT_DOUBLE_EQ(est->incomeLoad(), 0.0);
	EXPECT_DOUBLE_EQ(est->plr(), 0.0);
}

TEST(KalmanIncomeLoadEstimator, ZeroTotalSlotsKeepsState)
{
	auto est = IncomeLoadEstimatorFactory::make(KalmanIncomeLoadEstimatorConfig{});
	est->update(makeFrame(0, 0, 0, 0));
	EXPECT_DOUBLE_EQ(est->incomeLoad(), 0.0);
	EXPECT_DOUBLE_EQ(est->plr(), 0.0);
}

TEST(KalmanIncomeLoadEstimator, IncomeLoadNonNegative)
{
	auto est = IncomeLoadEstimatorFactory::make(KalmanIncomeLoadEstimatorConfig{});
	est->update(makeFrame(10, 9, 1, 0));
	EXPECT_GE(est->incomeLoad(), 0.0);
}

TEST(KalmanIncomeLoadEstimator, PlrClampedToUnitInterval)
{
	auto est = IncomeLoadEstimatorFactory::make(KalmanIncomeLoadEstimatorConfig{});
	for (int i = 0; i < 20; ++i)
	{
		est->update(makeFrame(10, 0, 0, 10));
		EXPECT_GE(est->plr(), 0.0);
		EXPECT_LE(est->plr(), 1.0);
	}
}

TEST(KalmanIncomeLoadEstimator, ResetReturnsToZero)
{
	auto est = IncomeLoadEstimatorFactory::make(KalmanIncomeLoadEstimatorConfig{});
	est->update(makeFrame(10, 0, 5, 5));
	est->reset();
	EXPECT_DOUBLE_EQ(est->incomeLoad(), 0.0);
	EXPECT_DOUBLE_EQ(est->plr(), 0.0);
}

TEST(KalmanIncomeLoadEstimator, ConvergesUnderSteadyInput)
{
	auto est = IncomeLoadEstimatorFactory::make(KalmanIncomeLoadEstimatorConfig{});
	for (int i = 0; i < 200; ++i)
		est->update(makeFrame(20, 10, 5, 5));

	const double settled = est->incomeLoad();
	EXPECT_GT(settled, 0.0);

	// Another step under the same input should not move the estimate much.
	est->update(makeFrame(20, 10, 5, 5));
	EXPECT_NEAR(est->incomeLoad(), settled, 0.05);
}
