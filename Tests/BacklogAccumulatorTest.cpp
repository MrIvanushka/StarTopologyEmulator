#include <gtest/gtest.h>

#include "StarTopologyEmulator/BacklogAccumulator/BacklogAccumulatorFactory.h"
#include "StarTopologyEmulator/Messages/BacklogReportMessage.h"

using namespace starTopologyEmulator;

TEST(BacklogAccumulator, EmptyAccumulatorProducesEmptyPlan)
{
	auto acc = BacklogAccumulatorFactory::make(/*bitsPerSlot=*/100);
	auto plan = acc->generateOperationPlan(/*frame=*/3, /*totalDedicatedSlots=*/5);
	ASSERT_NE(plan, nullptr);
	EXPECT_EQ(plan->frame(), 3u);
	EXPECT_TRUE(plan->allocations().empty());
}

TEST(BacklogAccumulator, HandleReportStoresLatestBacklogPerStation)
{
	auto acc = BacklogAccumulatorFactory::make(100);
	acc->handleReport(std::make_shared<BacklogReportMessage>(/*id=*/1, /*bits=*/200));
	EXPECT_EQ(acc->backlogFor(1), 200u);

	acc->handleReport(std::make_shared<BacklogReportMessage>(/*id=*/1, /*bits=*/50));
	EXPECT_EQ(acc->backlogFor(1), 50u);
}

TEST(BacklogAccumulator, SingleStationGetsFullBacklogWhenItFits)
{
	auto acc = BacklogAccumulatorFactory::make(/*bitsPerSlot=*/100);
	acc->handleReport(std::make_shared<BacklogReportMessage>(7, /*bits=*/250));

	// totalBits = 5 slots * 100 = 500, demand 250 fits.
	auto plan = acc->generateOperationPlan(0, /*totalDedicatedSlots=*/5);
	ASSERT_EQ(plan->allocations().size(), 1u);
	EXPECT_EQ(plan->allocations()[0].stationID, 7u);
	EXPECT_EQ(plan->allocations()[0].allocatedBits, 250u);
}

TEST(BacklogAccumulator, EqualBacklogsSplitProportionallyWhenCapacityIsLimited)
{
	auto acc = BacklogAccumulatorFactory::make(/*bitsPerSlot=*/100);
	acc->handleReport(std::make_shared<BacklogReportMessage>(1, 600));
	acc->handleReport(std::make_shared<BacklogReportMessage>(2, 600));

	// totalBits = 2 slots * 100 = 200; demand 1200 > capacity, so 100 each.
	auto plan = acc->generateOperationPlan(0, /*totalDedicatedSlots=*/2);
	ASSERT_EQ(plan->allocations().size(), 2u);
	EXPECT_EQ(plan->allocationFor(1), 100u);
	EXPECT_EQ(plan->allocationFor(2), 100u);
}

TEST(BacklogAccumulator, AsymmetricBacklogsSplitProportionally)
{
	auto acc = BacklogAccumulatorFactory::make(/*bitsPerSlot=*/100);
	acc->handleReport(std::make_shared<BacklogReportMessage>(1, 800));
	acc->handleReport(std::make_shared<BacklogReportMessage>(2, 200));

	// totalBits = 5 * 100 = 500; ratios 800/1000 and 200/1000 -> 400 and 100.
	auto plan = acc->generateOperationPlan(0, /*totalDedicatedSlots=*/5);
	EXPECT_EQ(plan->allocationFor(1), 400u);
	EXPECT_EQ(plan->allocationFor(2), 100u);
}

TEST(BacklogAccumulator, ZeroSlotsYieldZeroAllocations)
{
	auto acc = BacklogAccumulatorFactory::make(100);
	acc->handleReport(std::make_shared<BacklogReportMessage>(1, 500));

	auto plan = acc->generateOperationPlan(0, /*totalDedicatedSlots=*/0);
	ASSERT_EQ(plan->allocations().size(), 1u);
	EXPECT_EQ(plan->allocationFor(1), 0u);
}

TEST(BacklogAccumulator, ZeroBitsPerSlotYieldZeroAllocations)
{
	auto acc = BacklogAccumulatorFactory::make(/*bitsPerSlot=*/0);
	acc->handleReport(std::make_shared<BacklogReportMessage>(1, 500));

	auto plan = acc->generateOperationPlan(0, 5);
	EXPECT_EQ(plan->allocationFor(1), 0u);
}

TEST(BacklogAccumulator, AllZeroBacklogsYieldZeroAllocations)
{
	auto acc = BacklogAccumulatorFactory::make(100);
	acc->handleReport(std::make_shared<BacklogReportMessage>(1, 0));
	acc->handleReport(std::make_shared<BacklogReportMessage>(2, 0));

	auto plan = acc->generateOperationPlan(0, 5);
	EXPECT_EQ(plan->allocationFor(1), 0u);
	EXPECT_EQ(plan->allocationFor(2), 0u);
}
