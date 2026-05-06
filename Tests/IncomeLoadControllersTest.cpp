#include <cstdint>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Helpers.h"
#include "Mocks/MockDynamicFrameSettings.h"
#include "Mocks/MockIncomeStationsPredictor.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/IncomeLoadControllerFactory.h"

using namespace starTopologyEmulator;
using namespace tests;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

TEST(StaticIncomeLoadController, ReturnsConfigGivenAtConstruction)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	StarHubPlanMessage::BackoffConfig cfg;
	cfg.pTx = 0.42;
	cfg.baseWindow = 9;
	cfg.maxWindow = 64;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/4, /*currentFrame=*/3, /*targetFrame=*/5);

	EXPECT_DOUBLE_EQ(out.pTx, 0.42);
	EXPECT_EQ(out.baseWindow, 9);
	EXPECT_EQ(out.maxWindow, 64);
}

TEST(StaticIncomeLoadController, AlwaysQueriesPredictor)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	EXPECT_CALL(*predictor, estimateReadyUsers(/*current=*/3, /*target=*/5))
		.Times(1)
		.WillOnce(Return(0.0));

	StarHubPlanMessage::BackoffConfig cfg;
	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));
	controller->generate(/*plannedRaSlots=*/4, /*currentFrame=*/3, /*targetFrame=*/5);
}

TEST(HysteresisLoadController, NormalStateOnLowLoad)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1)));

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, HysteresisLoadControllerConfig{});

	const auto out = controller->generate(/*plannedRaSlots=*/5, /*currentFrame=*/10, /*targetFrame=*/11);
	EXPECT_DOUBLE_EQ(out.pTx, 1.0);
	EXPECT_EQ(out.baseWindow, 1u);
}

TEST(HysteresisLoadController, CriticalStateOnHighLoad)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	// 25 ready users with raSlots=1 -> holdLoad >> gHigh -> Critical state.
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(25.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1)));

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, HysteresisLoadControllerConfig{});

	const auto out = controller->generate(/*plannedRaSlots=*/1, /*currentFrame=*/10, /*targetFrame=*/11);
	EXPECT_DOUBLE_EQ(out.pTx, HysteresisLoadControllerConfig{}.pCritical);
	EXPECT_EQ(out.baseWindow, HysteresisLoadControllerConfig{}.backoffCriticalFrames);
}

TEST(TargetLoadController, NoPlanReturnsDefaultBackoff)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, TargetLoadControllerConfig{});
	const auto out = controller->generate(5, 0, 1);

	EXPECT_DOUBLE_EQ(out.pTx, StarHubPlanMessage::BackoffConfig{}.pTx);
	EXPECT_EQ(out.baseWindow, StarHubPlanMessage::BackoffConfig{}.baseWindow);
}

TEST(TargetLoadController, OutputAlwaysWithinConfiguredBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/2)));

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, TargetLoadControllerConfig{});
	const auto out = controller->generate(/*plannedRaSlots=*/5, /*currentFrame=*/10, /*targetFrame=*/11);

	TargetLoadControllerConfig refCfg;
	EXPECT_GE(out.pTx, refCfg.minProbability);
	EXPECT_LE(out.pTx, refCfg.maxProbability);
	EXPECT_GE(out.baseWindow, refCfg.minBackoffWindowFrames);
	EXPECT_LE(out.baseWindow, refCfg.maxBackoffWindowFrames);
}

TEST(SimpleMarginalUtilityBasedLoadController, GradientStepWithoutCollisionWeightRaisesP)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	SimpleMarginalUtilityBasedLoadControllerConfig cfg;
	cfg.weightThroughput = 1.0;
	cfg.weightCollision = 0.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;
	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));

	const auto out = controller->generate(/*plannedRaSlots=*/1, /*currentFrame=*/10, /*targetFrame=*/11);

	// At n=1, w_coll=0:  dU/dp = w_thr (constant). Step = 0.1 * 1.0 = 0.1.
	EXPECT_NEAR(out.pTx, 0.6, 1e-6);
}

TEST(SimpleMarginalUtilityBasedLoadController, MaxProbabilityStepClampsLargeMoves)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	SimpleMarginalUtilityBasedLoadControllerConfig cfg;
	cfg.weightThroughput = 1000.0;       // huge gradient
	cfg.weightCollision = 0.0;
	cfg.gradientStep = 1.0;
	cfg.maxProbabilityStep = 0.05;       // saturates here
	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));

	const auto out = controller->generate(1, 10, 11);
	// Move clamped to +0.05 from 0.5.
	EXPECT_NEAR(out.pTx, 0.55, 1e-9);
}

TEST(SimpleMarginalUtilityBasedLoadController, PreservesBackoffWindow)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/7)));

	auto controller = IncomeLoadControllerFactory::make(
		dfs,
		predictor,
		SimpleMarginalUtilityBasedLoadControllerConfig{});

	const auto out = controller->generate(1, 10, 11);
	EXPECT_EQ(out.baseWindow, 7u);
}

TEST(SimpleMarginalUtilityBasedLoadController, RespectsProbabilityBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(10, /*ra=*/5, /*pTx=*/0.95, /*baseWindow=*/1)));

	SimpleMarginalUtilityBasedLoadControllerConfig cfg;
	cfg.minProbability = 0.05;
	cfg.maxProbability = 0.9;            // forces clamp from 0.95 down
	cfg.weightThroughput = 1.0;
	cfg.weightCollision = 0.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;
	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));

	const auto out = controller->generate(1, 10, 11);
	EXPECT_LE(out.pTx, 0.9);
	EXPECT_GE(out.pTx, 0.05);
}
