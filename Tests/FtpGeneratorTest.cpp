#include <gtest/gtest.h>

#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/FtpGeneratorFactory.h"

using namespace starTopologyEmulator;

TEST(StaticFtpGenerator, ReturnsConfiguredCounts)
{
	auto gen = FtpGeneratorFactory::make(/*ra=*/5, /*yellow=*/3, /*operation=*/12);
	const auto cfg = gen->generate(0);
	EXPECT_EQ(cfg.randomAccessSlotsCountInFrame, 5u);
	EXPECT_EQ(cfg.yellowSlotsCountInFrame, 3u);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame, 12u);
}

TEST(StaticFtpGenerator, FrameNumberDoesNotAffectOutput)
{
	auto gen = FtpGeneratorFactory::make(7, 2, 9);
	const auto a = gen->generate(0);
	const auto b = gen->generate(100);
	const auto c = gen->generate(987654);

	EXPECT_EQ(a.randomAccessSlotsCountInFrame, b.randomAccessSlotsCountInFrame);
	EXPECT_EQ(b.randomAccessSlotsCountInFrame, c.randomAccessSlotsCountInFrame);
	EXPECT_EQ(a.yellowSlotsCountInFrame, c.yellowSlotsCountInFrame);
	EXPECT_EQ(a.onlineSlotsCountInFrame, c.onlineSlotsCountInFrame);
}

TEST(StaticFtpGenerator, AllZeroIsValid)
{
	auto gen = FtpGeneratorFactory::make(0, 0, 0);
	const auto cfg = gen->generate(42);
	EXPECT_EQ(cfg.randomAccessSlotsCountInFrame, 0u);
	EXPECT_EQ(cfg.yellowSlotsCountInFrame, 0u);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame, 0u);
}
