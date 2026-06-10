#include <cstdint>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Mocks/MockFtpGenerator.h"
#include "Mocks/MockIncomeLoadController.h"
#include "Mocks/MockIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyFactory.h"

using namespace starTopologyEmulator;
using namespace tests;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace
{

std::shared_ptr<NiceMock<MockIncomeLoadEstimator>> makeIdleEstimator()
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	ON_CALL(*est, incomeLoad()).WillByDefault(Return(0.0));
	ON_CALL(*est, plr()).WillByDefault(Return(0.0));
	return est;
}

} // namespace

TEST(SimpleStarHubStrategy, PlanFrameMatchesTargetFrame)
{
	auto est = makeIdleEstimator();
	auto strategy = StarHubStrategyFactory::make(est, StarHubStrategyConfig{});

	auto plan = strategy->generate(/*currentFrame=*/4, /*targetFrame=*/9);
	ASSERT_NE(plan, nullptr);
	EXPECT_EQ(plan->frame(), 9u);
}

TEST(SimpleStarHubStrategy, OutputsWithinConfiguredBounds)
{
	auto est = makeIdleEstimator();
	StarHubStrategyConfig cfg;
	auto strategy = StarHubStrategyFactory::make(est, std::move(cfg));

	StarHubStrategyConfig ref;
	auto plan = strategy->generate(0, 1);

	EXPECT_GE(plan->backoff().pTx, ref.minPTx);
	EXPECT_LE(plan->backoff().pTx, ref.maxPTx);
	EXPECT_GE(plan->randomAccessSlotsCountInFrame(), ref.minRaSlots);
	EXPECT_LE(plan->randomAccessSlotsCountInFrame(), ref.maxRaSlots);
}

TEST(SimpleStarHubStrategy, IdleEstimatorYieldsMaxPTxAndMinRaSlots)
{
	auto est = makeIdleEstimator();
	StarHubStrategyConfig cfg;
	auto strategy = StarHubStrategyFactory::make(est, std::move(cfg));

	StarHubStrategyConfig ref;
	auto plan = strategy->generate(0, 1);

	EXPECT_DOUBLE_EQ(plan->backoff().pTx, ref.maxPTx);
	EXPECT_EQ(plan->randomAccessSlotsCountInFrame(), ref.minRaSlots);
}

TEST(SimpleStarHubStrategy, HeavyLoadDrivesPTxToMin)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	// G == heavyLoadG (default 0.7), PLR == 1.0 -> stress == 1.
	ON_CALL(*est, incomeLoad()).WillByDefault(Return(5.0));
	ON_CALL(*est, plr()).WillByDefault(Return(1.0));

	StarHubStrategyConfig cfg;
	auto strategy = StarHubStrategyFactory::make(est, std::move(cfg));

	StarHubStrategyConfig ref;
	auto plan = strategy->generate(0, 1);
	EXPECT_DOUBLE_EQ(plan->backoff().pTx, ref.minPTx);
}

TEST(CommonStarHubStrategy, UsesFtpGeneratorOutputForRaSlots)
{
	StarHubPlanMessage::FtpConfig ftp{};
	ftp.randomAccessSlotsCountInFrame = 7;
	ftp.yellowSlotsCountInFrame = 2;
	ftp.onlineSlotsCountInFrame = 15;

	StarHubPlanMessage::BackoffConfig fixedBackoff;
	fixedBackoff.pTx = 0.33;
	fixedBackoff.baseWindow = 4;

	auto ftpGen = std::make_unique<NiceMock<MockFtpGenerator>>();
	ON_CALL(*ftpGen, generate(_, _)).WillByDefault(Return(ftp));

	auto controller = std::make_unique<NiceMock<MockIncomeLoadController>>();
	ON_CALL(*controller, generate(_, _, _)).WillByDefault(Return(fixedBackoff));

	auto strategy = StarHubStrategyFactory::make(std::move(ftpGen), std::move(controller));
	auto plan = strategy->generate(/*current=*/3, /*target=*/4);

	ASSERT_NE(plan, nullptr);
	EXPECT_EQ(plan->frame(), 4u);
	EXPECT_EQ(plan->randomAccessSlotsCountInFrame(), 7u);
	EXPECT_EQ(plan->yellowSlotsCountInFrame(), 2u);
	EXPECT_EQ(plan->onlineSlotsCountInFrame(), 15u);
}

TEST(CommonStarHubStrategy, PassesControllerBackoffThrough)
{
	StarHubPlanMessage::FtpConfig ftp{};
	ftp.randomAccessSlotsCountInFrame = 3;

	StarHubPlanMessage::BackoffConfig fixedBackoff;
	fixedBackoff.pTx = 0.123;
	fixedBackoff.baseWindow = 11;
	fixedBackoff.maxWindow = 64;

	auto ftpGen = std::make_unique<NiceMock<MockFtpGenerator>>();
	ON_CALL(*ftpGen, generate(_, _)).WillByDefault(Return(ftp));

	auto controller = std::make_unique<NiceMock<MockIncomeLoadController>>();
	ON_CALL(*controller, generate(_, _, _)).WillByDefault(Return(fixedBackoff));

	auto strategy = StarHubStrategyFactory::make(std::move(ftpGen), std::move(controller));
	auto plan = strategy->generate(0, 1);

	EXPECT_DOUBLE_EQ(plan->backoff().pTx, 0.123);
	EXPECT_EQ(plan->backoff().baseWindow, 11);
	EXPECT_EQ(plan->backoff().maxWindow, 64);
}

TEST(CommonStarHubStrategy, ForwardsTargetFrameToFtpGenerator)
{
	auto ftpGen = std::make_unique<NiceMock<MockFtpGenerator>>();
	EXPECT_CALL(*ftpGen, generate(/*current=*/4, /*target=*/9))
		.Times(1)
		.WillOnce(Return(StarHubPlanMessage::FtpConfig{}));

	auto controller = std::make_unique<NiceMock<MockIncomeLoadController>>();
	EXPECT_CALL(*controller, generate(_, /*current=*/4, /*target=*/9))
		.Times(1)
		.WillOnce(Return(StarHubPlanMessage::BackoffConfig{}));

	auto strategy = StarHubStrategyFactory::make(std::move(ftpGen), std::move(controller));
	auto plan = strategy->generate(/*current=*/4, /*target=*/9);
	EXPECT_EQ(plan->frame(), 9u);
}
