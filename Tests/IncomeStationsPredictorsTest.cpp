#include <cstdint>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Helpers.h"
#include "Mocks/MockDynamicFrameSettings.h"
#include "Mocks/MockFrameCalculator.h"
#include "Mocks/MockIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeStationsPredictor/IncomeStationsPredictorFactory.h"

using namespace starTopologyEmulator;
using namespace tests;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace
{

constexpr std::uint64_t kSlotsPerFrame = 10;
constexpr Timestamp kSlotDuration = 100;

std::shared_ptr<NiceMock<MockFrameCalculator>> makeLinearFrameCalcMock()
{
	auto fc = std::make_shared<NiceMock<MockFrameCalculator>>();
	ON_CALL(*fc, slotBeginTime(_, _)).WillByDefault(
		[](IFrameCalculator::FrameNum f, IFrameCalculator::SlotNum s) {
			return static_cast<Timestamp>(
				f * kSlotsPerFrame * kSlotDuration + s * kSlotDuration);
		});
	return fc;
}

} // namespace

TEST(CogorthyIncomeStationsPredictor, ZeroForEmptyHistory)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*est, incomeLoad()).WillByDefault(Return(0.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));

	auto pred = IncomeStationsPredictorFactory::make(est, dfs);
	EXPECT_DOUBLE_EQ(pred->estimateReadyUsers(/*current=*/0, /*target=*/1), 0.0);
}

TEST(CogorthyIncomeStationsPredictor, ReturnsZeroWhenTargetIsNotAfterCurrent)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	ON_CALL(*est, incomeLoad()).WillByDefault(Return(5.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		[](std::uint64_t f) -> std::shared_ptr<StarHubPlanMessage> {
			return f < 5 ? makePlan(f) : nullptr;
		});

	auto pred = IncomeStationsPredictorFactory::make(est, dfs);
	EXPECT_DOUBLE_EQ(pred->estimateReadyUsers(3, 3), 0.0);
	EXPECT_DOUBLE_EQ(pred->estimateReadyUsers(3, 2), 0.0);
}

TEST(CogorthyIncomeStationsPredictor, PositiveWhenLoadAndPlansPresent)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto plan = makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1);

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(5.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(10u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
	ON_CALL(*dfs, currentPlan(10)).WillByDefault(Return(plan));

	auto pred = IncomeStationsPredictorFactory::make(est, dfs);
	EXPECT_GT(pred->estimateReadyUsers(10, 11), 0.0);
}

TEST(CogorthyIncomeStationsPredictor, ZeroLoadGivesZeroEvenWithPlans)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto plan = makePlan(10);

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(0.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(10u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
	ON_CALL(*dfs, currentPlan(10)).WillByDefault(Return(plan));

	auto pred = IncomeStationsPredictorFactory::make(est, dfs);
	EXPECT_DOUBLE_EQ(pred->estimateReadyUsers(10, 11), 0.0);
}

TEST(LinearRegressionIncomeStationsPredictor, EmptyHistoryReturnsZero)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto fc = makeLinearFrameCalcMock();

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(0.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));

	auto pred = IncomeStationsPredictorFactory::make(
		est, dfs, fc, LinearRegressionIncomeStationsPredictorConfig{});

	EXPECT_DOUBLE_EQ(pred->estimateReadyUsers(0, 1), 0.0);
}

TEST(LinearRegressionIncomeStationsPredictor, SinglePointReturnsThatPoint)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto fc = makeLinearFrameCalcMock();
	auto plan = makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1);

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(5.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(10u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
	ON_CALL(*dfs, currentPlan(10)).WillByDefault(Return(plan));

	auto pred = IncomeStationsPredictorFactory::make(
		est, dfs, fc, LinearRegressionIncomeStationsPredictorConfig{});

	// historyReadyUsers = g * raSlots / aggressiveness = 5 * 5 / (1 * 2/(1+1)) = 25
	EXPECT_NEAR(pred->estimateReadyUsers(10, 11), 25.0, 1e-6);
}

TEST(LinearRegressionIncomeStationsPredictor, ConstantHistoryGivesStableExtrapolation)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto fc = makeLinearFrameCalcMock();

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(5.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		[](std::uint64_t f) -> std::shared_ptr<StarHubPlanMessage> {
			return f < 4 ? makePlan(f, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1) : nullptr;
		});

	auto pred = IncomeStationsPredictorFactory::make(
		est, dfs, fc, LinearRegressionIncomeStationsPredictorConfig{});

	for (std::uint64_t f = 0; f < 4; ++f)
		pred->estimateReadyUsers(f, f + 1);

	EXPECT_NEAR(pred->estimateReadyUsers(/*current=*/3, /*target=*/4), 25.0, 5.0);
}

TEST(LinearRegressionIncomeStationsPredictor, NeverReturnsNegative)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto fc = makeLinearFrameCalcMock();
	auto plan = makePlan(0);

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(0.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
	ON_CALL(*dfs, currentPlan(0)).WillByDefault(Return(plan));

	auto pred = IncomeStationsPredictorFactory::make(
		est, dfs, fc, LinearRegressionIncomeStationsPredictorConfig{});

	for (std::uint64_t f = 0; f < 5; ++f)
		EXPECT_GE(pred->estimateReadyUsers(f, f + 1), 0.0);
}

TEST(GreyModelIncomeStationsPredictor, EmptyHistoryReturnsZero)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(0.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));

	auto pred = IncomeStationsPredictorFactory::make(
		est, dfs, GreyModelIncomeStationsPredictorConfig{});

	EXPECT_DOUBLE_EQ(pred->estimateReadyUsers(0, 1), 0.0);
}

TEST(GreyModelIncomeStationsPredictor, ReturnsZeroWhenTargetIsNotAfterCurrent)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto plan = makePlan(/*frame=*/3, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1);

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(5.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
	ON_CALL(*dfs, currentPlan(3)).WillByDefault(Return(plan));

	auto pred = IncomeStationsPredictorFactory::make(
		est, dfs, GreyModelIncomeStationsPredictorConfig{});

	EXPECT_DOUBLE_EQ(pred->estimateReadyUsers(3, 3), 0.0);
	EXPECT_DOUBLE_EQ(pred->estimateReadyUsers(3, 2), 0.0);
}

TEST(GreyModelIncomeStationsPredictor, SinglePointReturnsThatPoint)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto plan = makePlan(/*frame=*/10, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1);

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(5.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(10u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
	ON_CALL(*dfs, currentPlan(10)).WillByDefault(Return(plan));

	auto pred = IncomeStationsPredictorFactory::make(
		est, dfs, GreyModelIncomeStationsPredictorConfig{});

	EXPECT_NEAR(pred->estimateReadyUsers(10, 11), 25.0, 1e-6);
}

TEST(GreyModelIncomeStationsPredictor, ConstantHistoryGivesStableExtrapolation)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(5.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		[](std::uint64_t f) -> std::shared_ptr<StarHubPlanMessage> {
			return f < 6 ? makePlan(f, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1) : nullptr;
		});

	GreyModelIncomeStationsPredictorConfig cfg;
	cfg.minHistory = 4;
	cfg.windowSize = 8;

	auto pred = IncomeStationsPredictorFactory::make(est, dfs, std::move(cfg));

	for (std::uint64_t f = 0; f < 6; ++f)
		pred->estimateReadyUsers(f, f + 1);

	EXPECT_NEAR(pred->estimateReadyUsers(/*current=*/5, /*target=*/6), 25.0, 5.0);
}

TEST(GreyModelIncomeStationsPredictor, NeverReturnsNegative)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
	auto plan = makePlan(0);

	ON_CALL(*est, incomeLoad()).WillByDefault(Return(0.0));
	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
	ON_CALL(*dfs, currentPlan(0)).WillByDefault(Return(plan));

	auto pred = IncomeStationsPredictorFactory::make(
		est, dfs, GreyModelIncomeStationsPredictorConfig{});

	for (std::uint64_t f = 0; f < 5; ++f)
		EXPECT_GE(pred->estimateReadyUsers(f, f + 1), 0.0);
}

TEST(GreyModelIncomeStationsPredictor, GrowingHistoryExtrapolatesUp)
{
	auto est = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
	auto dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();

	std::vector<double> loads = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
	std::uint64_t callIndex = 0;

	ON_CALL(*est, incomeLoad()).WillByDefault(
		[&]() { return loads[callIndex < loads.size() ? callIndex : loads.size() - 1]; });

	ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));
	ON_CALL(*dfs, currentPlan(_)).WillByDefault(
		[](std::uint64_t f) -> std::shared_ptr<StarHubPlanMessage> {
			return f < 6 ? makePlan(f, /*ra=*/1, /*pTx=*/1.0, /*baseWindow=*/1) : nullptr;
		});

	GreyModelIncomeStationsPredictorConfig cfg;
	cfg.minHistory = 4;
	cfg.windowSize = 8;

	auto pred = IncomeStationsPredictorFactory::make(est, dfs, std::move(cfg));

	double previous = -1.0;
	for (std::uint64_t f = 0; f < 6; ++f)
	{
		callIndex = f;
		previous = pred->estimateReadyUsers(f, f + 1);
	}

	EXPECT_GT(previous, 6.0);
}
