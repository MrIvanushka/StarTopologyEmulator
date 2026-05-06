#include <gtest/gtest.h>

#include "Helpers.h"
#include "StarTopologyEmulator/DynamicFrameSettingsFactory.h"

using namespace starTopologyEmulator;
using namespace tests;

TEST(DynamicFrameSettings, EmptyHasZeroEarliest)
{
	auto dfs = DynamicFrameSettingsFactory::make();
	EXPECT_EQ(dfs->earliestPlanNumber(), 0u);
	EXPECT_EQ(dfs->currentPlan(0), nullptr);
	EXPECT_EQ(dfs->currentPlan(42), nullptr);
}

TEST(DynamicFrameSettings, HandlePlanStoresAndRetrievesByFrame)
{
	auto dfs = DynamicFrameSettingsFactory::make();
	auto plan = makePlan(7, /*raSlots=*/3, /*pTx=*/0.5);

	dfs->handlePlan(plan);

	EXPECT_EQ(dfs->earliestPlanNumber(), 7u);
	auto retrieved = dfs->currentPlan(7);
	ASSERT_NE(retrieved, nullptr);
	EXPECT_EQ(retrieved->frame(), 7u);
	EXPECT_EQ(retrieved->randomAccessSlotsCountInFrame(), 3u);
	EXPECT_DOUBLE_EQ(retrieved->backoff().pTx, 0.5);
}

TEST(DynamicFrameSettings, MissingFrameReturnsNull)
{
	auto dfs = DynamicFrameSettingsFactory::make();
	dfs->handlePlan(makePlan(5));
	dfs->handlePlan(makePlan(10));

	EXPECT_NE(dfs->currentPlan(5), nullptr);
	EXPECT_NE(dfs->currentPlan(10), nullptr);
	EXPECT_EQ(dfs->currentPlan(7), nullptr);
	EXPECT_EQ(dfs->currentPlan(11), nullptr);
}

TEST(DynamicFrameSettings, EarliestIsFirstInsertedFront)
{
	auto dfs = DynamicFrameSettingsFactory::make();
	dfs->handlePlan(makePlan(3));
	dfs->handlePlan(makePlan(1));
	dfs->handlePlan(makePlan(2));

	EXPECT_EQ(dfs->earliestPlanNumber(), 3u);
}

TEST(DynamicFrameSettings, ClearOutdatedRemovesPlansBeforeFrame)
{
	auto dfs = DynamicFrameSettingsFactory::make();
	dfs->handlePlan(makePlan(1));
	dfs->handlePlan(makePlan(2));
	dfs->handlePlan(makePlan(3));
	dfs->handlePlan(makePlan(4));

	dfs->clearOutdated(3);

	EXPECT_EQ(dfs->currentPlan(1), nullptr);
	EXPECT_EQ(dfs->currentPlan(2), nullptr);
	EXPECT_NE(dfs->currentPlan(3), nullptr);
	EXPECT_NE(dfs->currentPlan(4), nullptr);
	EXPECT_EQ(dfs->earliestPlanNumber(), 3u);
}

TEST(DynamicFrameSettings, CapacityLimitEvictsOldest)
{
	auto dfs = DynamicFrameSettingsFactory::make(/*maxPlansStored=*/3);
	dfs->handlePlan(makePlan(10));
	dfs->handlePlan(makePlan(11));
	dfs->handlePlan(makePlan(12));
	dfs->handlePlan(makePlan(13));

	EXPECT_EQ(dfs->currentPlan(10), nullptr);
	EXPECT_NE(dfs->currentPlan(11), nullptr);
	EXPECT_NE(dfs->currentPlan(13), nullptr);
	EXPECT_EQ(dfs->earliestPlanNumber(), 11u);
}

TEST(DynamicFrameSettings, ClearOutdatedOnEmptyIsNoop)
{
	auto dfs = DynamicFrameSettingsFactory::make();
	dfs->clearOutdated(100);
	EXPECT_EQ(dfs->earliestPlanNumber(), 0u);
}
