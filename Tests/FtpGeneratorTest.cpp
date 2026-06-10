#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/BacklogFeedbackFtpGeneratorConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/FtpGeneratorFactory.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/LyapunovFtpGeneratorConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/MarginalUtilityFtpGeneratorConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/ServiceDelayFtpGeneratorConfig.h"

#include "Helpers.h"
#include "Mocks/MockBacklogAccumulator.h"
#include "Mocks/MockDynamicFrameSettings.h"
#include "Mocks/MockFrameCalculator.h"
#include "Mocks/MockIncomeStationsPredictor.h"

using namespace starTopologyEmulator;
using namespace tests;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;

TEST(StaticFtpGenerator, ReturnsConfiguredCounts)
{
	auto gen = FtpGeneratorFactory::make(/*ra=*/5, /*yellow=*/3, /*operation=*/12);
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.randomAccessSlotsCountInFrame, 5u);
	EXPECT_EQ(cfg.yellowSlotsCountInFrame, 3u);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame, 12u);
}

TEST(StaticFtpGenerator, FrameNumberDoesNotAffectOutput)
{
	auto gen = FtpGeneratorFactory::make(7, 2, 9);
	const auto a = gen->generate(0, 0);
	const auto b = gen->generate(100, 100);
	const auto c = gen->generate(987654, 987654);

	EXPECT_EQ(a.randomAccessSlotsCountInFrame, b.randomAccessSlotsCountInFrame);
	EXPECT_EQ(b.randomAccessSlotsCountInFrame, c.randomAccessSlotsCountInFrame);
	EXPECT_EQ(a.yellowSlotsCountInFrame, c.yellowSlotsCountInFrame);
	EXPECT_EQ(a.onlineSlotsCountInFrame, c.onlineSlotsCountInFrame);
}

TEST(StaticFtpGenerator, AllZeroIsValid)
{
	auto gen = FtpGeneratorFactory::make(0, 0, 0);
	const auto cfg = gen->generate(42, 42);
	EXPECT_EQ(cfg.randomAccessSlotsCountInFrame, 0u);
	EXPECT_EQ(cfg.yellowSlotsCountInFrame, 0u);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame, 0u);
}

// ---------------------------------------------------------------------------

class DynamicFtpGeneratorTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		dynSettings = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
		predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
		backlogAcc = std::make_shared<NiceMock<MockBacklogAccumulator>>();
		frameCalc = std::make_shared<NiceMock<MockFrameCalculator>>();

		ON_CALL(*backlogAcc, backlogs()).WillByDefault(ReturnRef(backlogMap));
		ON_CALL(*frameCalc, frameConfig())
			.WillByDefault(Return(makeFrameConfig(100, 100, 0, 1000)));
		ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(10.0));
		ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(makePlan(0, 5, 0.5, 1)));
	}

	std::shared_ptr<NiceMock<MockDynamicFrameSettings>> dynSettings;
	std::shared_ptr<NiceMock<MockIncomeStationsPredictor>> predictor;
	std::shared_ptr<NiceMock<MockBacklogAccumulator>> backlogAcc;
	std::shared_ptr<NiceMock<MockFrameCalculator>> frameCalc;
	std::map<StationID, std::uint64_t> backlogMap;

	std::unique_ptr<IFtpGenerator> makeBacklogFeedback(BacklogFeedbackFtpGeneratorConfig cfg = {})
	{
		return FtpGeneratorFactory::makeBacklogFeedback(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeServiceDelay(ServiceDelayFtpGeneratorConfig cfg = {})
	{
		return FtpGeneratorFactory::makeServiceDelay(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeLyapunov(LyapunovFtpGeneratorConfig cfg = {})
	{
		return FtpGeneratorFactory::makeLyapunov(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeMarginalUtility(
		MarginalUtilityFtpGeneratorConfig cfg = F3AlphaFairUtilityConfig{})
	{
		return FtpGeneratorFactory::makeMarginalUtility(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeF1Linear(F1LinearUtilityConfig cfg = {})
	{
		return FtpGeneratorFactory::makeF1Linear(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeF2Logarithmic(F2LogarithmicUtilityConfig cfg = {})
	{
		return FtpGeneratorFactory::makeF2Logarithmic(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeF3AlphaFair(F3AlphaFairUtilityConfig cfg = {})
	{
		return FtpGeneratorFactory::makeF3AlphaFair(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeF4Sigmoidal(F4SigmoidalUtilityConfig cfg = {})
	{
		return FtpGeneratorFactory::makeF4Sigmoidal(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeF5HardDeadline(F5HardDeadlineUtilityConfig cfg = {})
	{
		return FtpGeneratorFactory::makeF5HardDeadline(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeF6CostOfDelay(F6CostOfDelayUtilityConfig cfg = {})
	{
		return FtpGeneratorFactory::makeF6CostOfDelay(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeF7QuadraticBacklog(F7QuadraticBacklogUtilityConfig cfg = {})
	{
		return FtpGeneratorFactory::makeF7QuadraticBacklog(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}

	std::unique_ptr<IFtpGenerator> makeF8Ces(F8CesUtilityConfig cfg = {})
	{
		return FtpGeneratorFactory::makeF8Ces(
			dynSettings, predictor, backlogAcc, frameCalc, std::move(cfg));
	}
};

// ---------------------------------------------------------------------------
// BacklogFeedbackFtpGenerator
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, BacklogFeedback_SlotSumEqualsTotal)
{
	auto gen = makeBacklogFeedback();
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, BacklogFeedback_YellowSlotsPreserved)
{
	BacklogFeedbackFtpGeneratorConfig cfg;
	cfg.yellowSlots = 7;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeBacklogFeedback(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.yellowSlotsCountInFrame, 7u);
}

TEST_F(DynamicFtpGeneratorTest, BacklogFeedback_BaselineFromR0)
{
	BacklogFeedbackFtpGeneratorConfig cfg;
	cfg.R0 = 20.0;
	cfg.kJ = 0.0;
	cfg.kQ = 0.0;
	cfg.deltaR = 1000.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeBacklogFeedback(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 20u);
}

TEST_F(DynamicFtpGeneratorTest, BacklogFeedback_RaClampedToMax_WhenRaDemandHigh)
{
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(1000.0));

	BacklogFeedbackFtpGeneratorConfig cfg;
	cfg.R0 = 20.0;
	cfg.kJ = 1.0;
	cfg.jStar = 10.0;
	cfg.kQ = 0.1;
	cfg.qStar = 100.0;
	cfg.deltaR = 1000.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeBacklogFeedback(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 50u);
}

TEST_F(DynamicFtpGeneratorTest, BacklogFeedback_RaClampedToMin_WhenDaBacklogHigh)
{
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(10.0));
	backlogMap[1] = 1'000'000'000ULL;

	BacklogFeedbackFtpGeneratorConfig cfg;
	cfg.R0 = 20.0;
	cfg.kJ = 1.0;
	cfg.jStar = 10.0;
	cfg.kQ = 1.0;
	cfg.qStar = 100.0;
	cfg.deltaR = 1000.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeBacklogFeedback(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 5u);
}

TEST_F(DynamicFtpGeneratorTest, BacklogFeedback_StepLimitedPerFrame)
{
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(1000.0));

	BacklogFeedbackFtpGeneratorConfig cfg;
	cfg.R0 = 5.0;
	cfg.kJ = 1.0;
	cfg.jStar = 0.0;
	cfg.kQ = 0.0;
	cfg.deltaR = 1.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeBacklogFeedback(std::move(cfg));
	const auto first = gen->generate(0, 0);
	const auto second = gen->generate(1, 1);

	EXPECT_EQ(second.randomAccessSlotsCountInFrame,
		first.randomAccessSlotsCountInFrame + 1u);
}

// ---------------------------------------------------------------------------
// ServiceDelayFtpGenerator
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, ServiceDelay_SlotSumEqualsTotal)
{
	auto gen = makeServiceDelay();
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, ServiceDelay_YellowSlotsPreserved)
{
	ServiceDelayFtpGeneratorConfig cfg;
	cfg.yellowSlots = 8;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeServiceDelay(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.yellowSlotsCountInFrame, 8u);
}

TEST_F(DynamicFtpGeneratorTest, ServiceDelay_BoundsRespected)
{
	ServiceDelayFtpGeneratorConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeServiceDelay(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, ServiceDelay_ZeroBacklogFavorsMaxRa)
{
	ServiceDelayFtpGeneratorConfig cfg;
	cfg.d0 = 0.0;
	cfg.lambdaRatio = 1.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeServiceDelay(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 50u);
}

TEST_F(DynamicFtpGeneratorTest, ServiceDelay_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeServiceDelay();
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// LyapunovFtpGenerator
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, Lyapunov_SlotSumEqualsTotal)
{
	auto gen = makeLyapunov();
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, Lyapunov_YellowSlotsPreserved)
{
	LyapunovFtpGeneratorConfig cfg;
	cfg.yellowSlots = 6;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeLyapunov(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.yellowSlotsCountInFrame, 6u);
}

TEST_F(DynamicFtpGeneratorTest, Lyapunov_ZeroRaQueue_HighDa_FavorsMinRa)
{
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));
	backlogMap[1] = 10'000'000ULL;

	LyapunovFtpGeneratorConfig cfg;
	cfg.V = 0.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeLyapunov(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 5u);
}

TEST_F(DynamicFtpGeneratorTest, Lyapunov_HighRaQueue_ZeroDa_FavorsMaxRa)
{
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(5.0));

	LyapunovFtpGeneratorConfig cfg;
	cfg.V = 0.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeLyapunov(std::move(cfg));
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 50u);
}

TEST_F(DynamicFtpGeneratorTest, Lyapunov_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeLyapunov();
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// MarginalUtilityFtpGenerator — F3 Alpha-fair (backward-compatible tests)
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, MarginalUtility_SlotSumEqualsTotal)
{
	auto gen = makeMarginalUtility();
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, MarginalUtility_YellowSlotsPreserved)
{
	F3AlphaFairUtilityConfig cfg;
	cfg.yellowSlots = 9;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.yellowSlotsCountInFrame, 9u);
}

TEST_F(DynamicFtpGeneratorTest, MarginalUtility_BoundsRespected)
{
	F3AlphaFairUtilityConfig cfg;
	cfg.raMin = 8;
	cfg.raMax = 40;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 8u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 40u);
}

TEST_F(DynamicFtpGeneratorTest, MarginalUtility_ZeroBacklogNoColl_FavorsMaxRa)
{
	F3AlphaFairUtilityConfig cfg;
	cfg.wColl = 0.0;
	cfg.wAcq = 1.0;
	cfg.wAuth = 0.0;
	cfg.wB = 0.0;
	cfg.wD = 0.0;
	cfg.alphaFair = 1.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 50u);
}

TEST_F(DynamicFtpGeneratorTest, MarginalUtility_HugeCollisionPenalty_FavorsMinRa)
{
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(1000.0));

	F3AlphaFairUtilityConfig cfg;
	cfg.wColl = 10000.0;
	cfg.wAcq = 1.0;
	cfg.wAuth = 0.0;
	cfg.wB = 0.0;
	cfg.wD = 0.0;
	cfg.alphaFair = 1.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 5u);
}

TEST_F(DynamicFtpGeneratorTest, MarginalUtility_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeMarginalUtility();
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// MarginalUtilityFtpGenerator — F1 Linear
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, F1Linear_SlotSumEqualsTotal)
{
	auto gen = makeMarginalUtility(F1LinearUtilityConfig{});
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, F1Linear_BoundsRespected)
{
	F1LinearUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, F1Linear_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeMarginalUtility(F1LinearUtilityConfig{});
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// MarginalUtilityFtpGenerator — F2 Logarithmic
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, F2Logarithmic_SlotSumEqualsTotal)
{
	auto gen = makeMarginalUtility(F2LogarithmicUtilityConfig{});
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, F2Logarithmic_BoundsRespected)
{
	F2LogarithmicUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

// ---------------------------------------------------------------------------
// MarginalUtilityFtpGenerator — F4 Sigmoidal
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, F4Sigmoidal_SlotSumEqualsTotal)
{
	auto gen = makeMarginalUtility(F4SigmoidalUtilityConfig{});
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, F4Sigmoidal_BoundsRespected)
{
	F4SigmoidalUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, F4Sigmoidal_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeMarginalUtility(F4SigmoidalUtilityConfig{});
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// MarginalUtilityFtpGenerator — F5 Hard Deadline
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, F5HardDeadline_SlotSumEqualsTotal)
{
	auto gen = makeMarginalUtility(F5HardDeadlineUtilityConfig{});
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, F5HardDeadline_BoundsRespected)
{
	F5HardDeadlineUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, F5HardDeadline_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeMarginalUtility(F5HardDeadlineUtilityConfig{});
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// MarginalUtilityFtpGenerator — F6 Cost-of-Delay
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, F6CostOfDelay_SlotSumEqualsTotal)
{
	auto gen = makeMarginalUtility(F6CostOfDelayUtilityConfig{});
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, F6CostOfDelay_BoundsRespected)
{
	F6CostOfDelayUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, F6CostOfDelay_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeMarginalUtility(F6CostOfDelayUtilityConfig{});
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// MarginalUtilityFtpGenerator — F7 Quadratic Backlog
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, F7QuadraticBacklog_SlotSumEqualsTotal)
{
	auto gen = makeMarginalUtility(F7QuadraticBacklogUtilityConfig{});
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, F7QuadraticBacklog_BoundsRespected)
{
	F7QuadraticBacklogUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, F7QuadraticBacklog_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeMarginalUtility(F7QuadraticBacklogUtilityConfig{});
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// MarginalUtilityFtpGenerator — F8 CES
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, F8Ces_SlotSumEqualsTotal)
{
	auto gen = makeMarginalUtility(F8CesUtilityConfig{});
	const auto cfg = gen->generate(0, 0);
	const auto total = cfg.onlineSlotsCountInFrame
		+ cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, F8Ces_CobbDouglas_SlotSumEqualsTotal)
{
	F8CesUtilityConfig cfg;
	cfg.rho = 0.0;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

TEST_F(DynamicFtpGeneratorTest, F8Ces_BoundsRespected)
{
	F8CesUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeMarginalUtility(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, F8Ces_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeMarginalUtility(F8CesUtilityConfig{});
	const auto result = gen->generate(0, 0);
	const auto total = result.onlineSlotsCountInFrame
		+ result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame;
	EXPECT_EQ(total, 100u);
}

// ---------------------------------------------------------------------------
// Individual generators — makeF1Linear ... makeF8Ces
// ---------------------------------------------------------------------------

TEST_F(DynamicFtpGeneratorTest, IndividualF1Linear_SlotSumEqualsTotal)
{
	auto gen = makeF1Linear();
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame + cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF1Linear_BoundsRespected)
{
	F1LinearUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeF1Linear(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF1Linear_NullPlanDoesNotCrash)
{
	ON_CALL(*dynSettings, currentPlan(_)).WillByDefault(Return(nullptr));

	auto gen = makeF1Linear();
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.onlineSlotsCountInFrame + result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF2Logarithmic_SlotSumEqualsTotal)
{
	auto gen = makeF2Logarithmic();
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame + cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF2Logarithmic_BoundsRespected)
{
	F2LogarithmicUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeF2Logarithmic(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF3AlphaFair_SlotSumEqualsTotal)
{
	auto gen = makeF3AlphaFair();
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame + cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF3AlphaFair_ZeroBacklogNoColl_FavorsMaxRa)
{
	F3AlphaFairUtilityConfig cfg;
	cfg.wColl = 0.0;
	cfg.wAcq = 1.0;
	cfg.wB = 0.0;
	cfg.wD = 0.0;
	cfg.raMin = 5;
	cfg.raMax = 50;

	auto gen = makeF3AlphaFair(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.randomAccessSlotsCountInFrame, 50u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF4Sigmoidal_SlotSumEqualsTotal)
{
	auto gen = makeF4Sigmoidal();
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame + cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF4Sigmoidal_BoundsRespected)
{
	F4SigmoidalUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeF4Sigmoidal(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF5HardDeadline_SlotSumEqualsTotal)
{
	auto gen = makeF5HardDeadline();
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame + cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF5HardDeadline_BoundsRespected)
{
	F5HardDeadlineUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeF5HardDeadline(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF6CostOfDelay_SlotSumEqualsTotal)
{
	auto gen = makeF6CostOfDelay();
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame + cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF6CostOfDelay_BoundsRespected)
{
	F6CostOfDelayUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeF6CostOfDelay(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF7QuadraticBacklog_SlotSumEqualsTotal)
{
	auto gen = makeF7QuadraticBacklog();
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame + cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF7QuadraticBacklog_BoundsRespected)
{
	F7QuadraticBacklogUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeF7QuadraticBacklog(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF8Ces_SlotSumEqualsTotal)
{
	auto gen = makeF8Ces();
	const auto cfg = gen->generate(0, 0);
	EXPECT_EQ(cfg.onlineSlotsCountInFrame + cfg.yellowSlotsCountInFrame
		+ cfg.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF8Ces_CobbDouglas_SlotSumEqualsTotal)
{
	F8CesUtilityConfig cfg;
	cfg.rho = 0.0;

	auto gen = makeF8Ces(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_EQ(result.onlineSlotsCountInFrame + result.yellowSlotsCountInFrame
		+ result.randomAccessSlotsCountInFrame, 100u);
}

TEST_F(DynamicFtpGeneratorTest, IndividualF8Ces_BoundsRespected)
{
	F8CesUtilityConfig cfg;
	cfg.raMin = 10;
	cfg.raMax = 30;

	auto gen = makeF8Ces(cfg);
	const auto result = gen->generate(0, 0);
	EXPECT_GE(result.randomAccessSlotsCountInFrame, 10u);
	EXPECT_LE(result.randomAccessSlotsCountInFrame, 30u);
}
