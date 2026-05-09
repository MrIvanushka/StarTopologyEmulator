#include <cstdint>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Helpers.h"
#include "Mocks/MockDynamicFrameSettings.h"
#include "Mocks/MockIncomeStationsPredictor.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AlphaFairLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/CollisionBudgetLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/EnergyAwareLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/GreyModelAdaptiveBackoffControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/IncomeLoadControllerFactory.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/LogBarrierLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/PiLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/PseudoBayesianLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/RiskSensitiveLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/SimpleMarginalUtilityBasedLoadControllerConfig.h"

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

TEST(PiLoadController, NoLoadErrorKeepsProbabilityUnchanged)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(5.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.9, /*baseWindow=*/1)));

	PiLoadControllerConfig cfg;
	cfg.gTarget = 0.9;
	cfg.kP = 1.0;
	cfg.kI = 0.0;
	cfg.alpha = 1.0;
	cfg.minProbability = 0.0;
	cfg.maxProbability = 1.0;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/5, /*currentFrame=*/10, /*targetFrame=*/11);

	EXPECT_NEAR(out.pTx, 0.9, 1e-9);
}

TEST(PiLoadController, OverloadDecreasesProbability)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(25.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1)));

	PiLoadControllerConfig cfg;
	cfg.gTarget = 0.9;
	cfg.kP = 0.5;
	cfg.kI = 0.0;
	cfg.alpha = 1.0;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/1, /*currentFrame=*/10, /*targetFrame=*/11);

	EXPECT_LT(out.pTx, 1.0);
}

TEST(PiLoadController, UnderloadIncreasesProbability)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.2, /*baseWindow=*/1)));

	PiLoadControllerConfig cfg;
	cfg.gTarget = 0.9;
	cfg.kP = 1.0;
	cfg.kI = 0.0;
	cfg.alpha = 1.0;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/5, /*currentFrame=*/10, /*targetFrame=*/11);

	EXPECT_GT(out.pTx, 0.2);
}

TEST(PiLoadController, RespectsMaxProbabilityStep)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(1000.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	PiLoadControllerConfig cfg;
	cfg.gTarget = 0.5;
	cfg.kP = 100.0;
	cfg.kI = 0.0;
	cfg.alpha = 1.0;
	cfg.maxProbabilityStep = 0.05;
	cfg.minProbability = 0.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/1, /*currentFrame=*/10, /*targetFrame=*/11);

	EXPECT_NEAR(out.pTx, 0.45, 1e-9);
}

TEST(PiLoadController, IntegralAccumulatesPersistentError)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.1, /*baseWindow=*/1)));

	PiLoadControllerConfig cfg;
	cfg.gTarget = 0.9;
	cfg.kP = 0.0;
	cfg.kI = 0.1;
	cfg.alpha = 1.0;
	cfg.integralWindowFrames = 1000;
	cfg.maxProbabilityStep = 1.0;
	cfg.minProbability = 0.0;
	cfg.maxProbability = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));

	const auto first = controller->generate(/*plannedRaSlots=*/5, 10, 11);
	const auto second = controller->generate(/*plannedRaSlots=*/5, 10, 12);
	const auto third = controller->generate(/*plannedRaSlots=*/5, 10, 13);

	EXPECT_GT(second.pTx, first.pTx);
	EXPECT_GT(third.pTx, second.pTx);
}

TEST(PiLoadController, OutputAlwaysWithinConfiguredBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(1000.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/2)));

	PiLoadControllerConfig cfg;
	cfg.minProbability = 0.05;
	cfg.maxProbability = 0.8;
	cfg.kP = 100.0;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/5, /*currentFrame=*/10, /*targetFrame=*/11);

	EXPECT_GE(out.pTx, 0.05);
	EXPECT_LE(out.pTx, 0.8);
}

TEST(PiLoadController, NoCurrentPlanReusesLastOutput)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));

	PiLoadControllerConfig cfg;
	cfg.gTarget = 0.9;
	cfg.kP = 0.0;
	cfg.kI = 0.0;
	cfg.backoffTemplate.pTx = 0.42;
	cfg.backoffTemplate.baseWindow = 7;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/5, 0, 1);

	EXPECT_DOUBLE_EQ(out.pTx, 0.42);
	EXPECT_EQ(out.baseWindow, 7u);
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

TEST(PseudoBayesianLoadController, OptimalPTxIsOneOverN)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(5.0));

	PseudoBayesianLoadControllerConfig cfg;
	cfg.minProbability = 0.0;
	cfg.maxProbability = 1.0;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/4, /*currentFrame=*/3, /*targetFrame=*/5);

	EXPECT_NEAR(out.pTx, 0.2, 1e-9);
}

TEST(PseudoBayesianLoadController, FewActiveStationsSaturateAtMaxProbability)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.5));

	PseudoBayesianLoadControllerConfig cfg;
	cfg.maxProbability = 0.8;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));
	const auto out = controller->generate(4, 3, 5);

	EXPECT_DOUBLE_EQ(out.pTx, 0.8);
}

TEST(PseudoBayesianLoadController, ManyActiveStationsHitMinProbability)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(10000.0));

	PseudoBayesianLoadControllerConfig cfg;
	cfg.minProbability = 0.001;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));
	const auto out = controller->generate(4, 3, 5);

	EXPECT_GE(out.pTx, 0.001);
}

TEST(PseudoBayesianLoadController, BackoffTemplateIsForwarded)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(5.0));

	PseudoBayesianLoadControllerConfig cfg;
	cfg.backoffTemplate.baseWindow = 11;
	cfg.backoffTemplate.maxWindow = 64;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));
	const auto out = controller->generate(4, 3, 5);

	EXPECT_EQ(out.baseWindow, 11u);
	EXPECT_EQ(out.maxWindow, 64u);
}

TEST(CollisionBudgetLoadController, BelowBudgetActsLikeThroughputOnly)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(2.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.4, /*baseWindow=*/1)));

	CollisionBudgetLoadControllerConfig cfg;
	cfg.collisionBudget = 0.99;
	cfg.gradientStep = 0.1;
	cfg.lagrangianStep = 0.5;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/5, 10, 11);

	EXPECT_GT(out.pTx, 0.4);
}

TEST(CollisionBudgetLoadController, OverBudgetReducesProbabilityViaLagrangian)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(20.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	CollisionBudgetLoadControllerConfig cfg;
	cfg.collisionBudget = 0.05;
	cfg.gradientStep = 0.05;
	cfg.lagrangianStep = 1.0;
	cfg.maxProbabilityStep = 0.5;
	cfg.minProbability = 0.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));

	double last = 0.5;
	for (int i = 0; i < 20; ++i)
	{
		const auto out = controller->generate(5, 10, 11 + i);
		last = out.pTx;
	}

	EXPECT_LT(last, 0.5);
}

TEST(CollisionBudgetLoadController, RespectsProbabilityBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(50.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	CollisionBudgetLoadControllerConfig cfg;
	cfg.minProbability = 0.05;
	cfg.maxProbability = 0.7;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	for (int i = 0; i < 30; ++i)
	{
		const auto out = controller->generate(5, 10, 11 + i);
		EXPECT_GE(out.pTx, 0.05);
		EXPECT_LE(out.pTx, 0.7);
	}
}

TEST(GreyModelAdaptiveBackoffController, ZeroPredictedLoadFallsBackToTemplate)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	GreyModelAdaptiveBackoffControllerConfig cfg;
	cfg.backoffTemplate.baseWindow = 8;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));

	const auto first = controller->generate(/*plannedRaSlots=*/5, 10, 11);
	EXPECT_EQ(first.baseWindow, 8u);
}

TEST(GreyModelAdaptiveBackoffController, ConstantHistoryProducesStableBackoff)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(5.0));

	GreyModelAdaptiveBackoffControllerConfig cfg;
	cfg.gTarget = 0.9;
	cfg.historySize = 6;
	cfg.minHistoryForPrediction = 4;
	cfg.minBackoffWindow = 1;
	cfg.maxBackoffWindow = 64;
	cfg.backoffTemplate.pTx = 1.0;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));

	StarHubPlanMessage::BackoffConfig last;
	for (int i = 0; i < 8; ++i)
		last = controller->generate(/*plannedRaSlots=*/5, 10, 11 + i);

	const auto next = controller->generate(/*plannedRaSlots=*/5, 10, 100);
	EXPECT_EQ(next.baseWindow, last.baseWindow);
}

TEST(GreyModelAdaptiveBackoffController, GrowingLoadIncreasesBackoff)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	int call = 0;
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault([&call](std::uint64_t, std::uint64_t) {
		return 1.0 + 1.5 * static_cast<double>(call++);
	});

	GreyModelAdaptiveBackoffControllerConfig cfg;
	cfg.gTarget = 0.5;
	cfg.historySize = 6;
	cfg.minHistoryForPrediction = 4;
	cfg.minBackoffWindow = 1;
	cfg.maxBackoffWindow = 200;
	cfg.backoffTemplate.pTx = 1.0;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));

	StarHubPlanMessage::BackoffConfig early;
	for (int i = 0; i < 4; ++i)
		early = controller->generate(/*plannedRaSlots=*/5, 10, 11 + i);

	StarHubPlanMessage::BackoffConfig late;
	for (int i = 0; i < 4; ++i)
		late = controller->generate(/*plannedRaSlots=*/5, 10, 20 + i);

	EXPECT_GT(late.baseWindow, early.baseWindow);
}

TEST(GreyModelAdaptiveBackoffController, RespectsBackoffBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(1000.0));

	GreyModelAdaptiveBackoffControllerConfig cfg;
	cfg.minBackoffWindow = 4;
	cfg.maxBackoffWindow = 12;

	auto controller = IncomeLoadControllerFactory::make(predictor, std::move(cfg));
	for (int i = 0; i < 10; ++i)
	{
		const auto out = controller->generate(/*plannedRaSlots=*/5, 10, 11 + i);
		EXPECT_GE(out.baseWindow, 4u);
		EXPECT_LE(out.baseWindow, 12u);
	}
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

TEST(LogBarrierLoadController, NormalLoadIsNearThroughputOnly)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(2.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.3, /*baseWindow=*/1)));

	LogBarrierLoadControllerConfig cfg;
	cfg.weightThroughput = 1.0;
	cfg.weightCollision = 0.01;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/1, 10, 11);

	EXPECT_GT(out.pTx, 0.3);
}

TEST(LogBarrierLoadController, BarrierKicksInNearCollisionSaturation)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(50.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.95, /*baseWindow=*/1)));

	LogBarrierLoadControllerConfig cfg;
	cfg.weightThroughput = 1.0;
	cfg.weightCollision = 1.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;
	cfg.minProbability = 0.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/1, 10, 11);

	EXPECT_LT(out.pTx, 0.95);
}

TEST(LogBarrierLoadController, RespectsProbabilityBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(100.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	LogBarrierLoadControllerConfig cfg;
	cfg.minProbability = 0.05;
	cfg.maxProbability = 0.7;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	for (int i = 0; i < 20; ++i)
	{
		const auto out = controller->generate(1, 10, 11 + i);
		EXPECT_GE(out.pTx, 0.05);
		EXPECT_LE(out.pTx, 0.7);
	}
}

TEST(EnergyAwareLoadController, EnergyPenaltyShiftsOptimumDown)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(2.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	EnergyAwareLoadControllerConfig cfg;
	cfg.weightThroughput = 1.0;
	cfg.weightEnergy = 5.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;
	cfg.minProbability = 0.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/1, 10, 11);

	EXPECT_LT(out.pTx, 0.5);
}

TEST(EnergyAwareLoadController, ZeroEnergyWeightMatchesThroughputOnly)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	EnergyAwareLoadControllerConfig cfg;
	cfg.weightThroughput = 1.0;
	cfg.weightEnergy = 0.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(1, 10, 11);

	EXPECT_NEAR(out.pTx, 0.6, 1e-6);
}

TEST(EnergyAwareLoadController, RespectsProbabilityBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(50.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	EnergyAwareLoadControllerConfig cfg;
	cfg.minProbability = 0.05;
	cfg.maxProbability = 0.7;
	cfg.weightEnergy = 100.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	for (int i = 0; i < 20; ++i)
	{
		const auto out = controller->generate(1, 10, 11 + i);
		EXPECT_GE(out.pTx, 0.05);
		EXPECT_LE(out.pTx, 0.7);
	}
}

TEST(AlphaFairLoadController, AlphaZeroIsEquivalentToThroughputOnly)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(0.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	AlphaFairLoadControllerConfig cfg;
	cfg.alpha = 0.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(1, 10, 11);

	EXPECT_NEAR(out.pTx, 0.6, 1e-6);
}

TEST(AlphaFairLoadController, AlphaOneIsProportionalFairness)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(2.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.3, /*baseWindow=*/1)));

	AlphaFairLoadControllerConfig cfg;
	cfg.alpha = 1.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(1, 10, 11);

	EXPECT_GT(out.pTx, 0.3);
}

TEST(AlphaFairLoadController, RespectsProbabilityBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(5.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	AlphaFairLoadControllerConfig cfg;
	cfg.alpha = 2.0;
	cfg.minProbability = 0.05;
	cfg.maxProbability = 0.7;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	for (int i = 0; i < 20; ++i)
	{
		const auto out = controller->generate(1, 10, 11 + i);
		EXPECT_GE(out.pTx, 0.05);
		EXPECT_LE(out.pTx, 0.7);
	}
}

TEST(RiskSensitiveLoadController, HighBetaPushesProbabilityDown)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(5.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	RiskSensitiveLoadControllerConfig cfg;
	cfg.beta = 5.0;
	cfg.collisionPenalty = 1.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;
	cfg.minProbability = 0.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(/*plannedRaSlots=*/1, 10, 11);

	EXPECT_LT(out.pTx, 0.5);
}

TEST(RiskSensitiveLoadController, LowRiskAversionPermitsAggressiveStep)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(2.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.3, /*baseWindow=*/1)));

	RiskSensitiveLoadControllerConfig cfg;
	cfg.beta = 0.1;
	cfg.collisionPenalty = 1.0;
	cfg.gradientStep = 0.1;
	cfg.maxProbabilityStep = 1.0;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	const auto out = controller->generate(1, 10, 11);

	EXPECT_GT(out.pTx, 0.3);
}

TEST(RiskSensitiveLoadController, RespectsProbabilityBounds)
{
	auto predictor = std::make_shared<NiceMock<MockIncomeStationsPredictor>>();
	ON_CALL(*predictor, estimateReadyUsers(_, _)).WillByDefault(Return(20.0));

	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		Return(makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/0.5, /*baseWindow=*/1)));

	RiskSensitiveLoadControllerConfig cfg;
	cfg.minProbability = 0.05;
	cfg.maxProbability = 0.8;

	auto controller = IncomeLoadControllerFactory::make(dfs, predictor, std::move(cfg));
	for (int i = 0; i < 20; ++i)
	{
		const auto out = controller->generate(1, 10, 11 + i);
		EXPECT_GE(out.pTx, 0.05);
		EXPECT_LE(out.pTx, 0.8);
	}
}
