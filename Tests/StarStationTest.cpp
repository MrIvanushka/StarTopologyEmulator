#include <cstdint>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Helpers.h"
#include "Mocks/MockDynamicFrameSettings.h"
#include "Mocks/MockFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/Messages/BacklogReportMessage.h"
#include "StarTopologyEmulator/Messages/OperationPlanMessage.h"
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"
#include "StarTopologyEmulator/Stations/StarStationFactory.h"
#include "StarTopologyEmulator/TerminalState.h"
#include "StarTopologyEmulator/TrafficProfile/CbrTrafficProfileConfig.h"
#include "StarTopologyEmulator/TrafficProfile/TrafficProfileFactory.h"

using namespace starTopologyEmulator;
using namespace tests;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace
{

constexpr std::uint64_t kSlotsPerFrame = 10;
constexpr Timestamp kSlotDuration = 100;

struct StationFixture
{
	std::vector<std::pair<Timestamp, std::shared_ptr<IMessage>>> sent;
	std::mt19937 rng{ 12345 };

	NiceMock<MockFrameCalculator>* frameCalc = nullptr;
	NiceMock<MockDynamicFrameSettings>* dfs = nullptr;

	std::shared_ptr<IStarStation> makeStation(
		StationID id = 7,
		int messagesNeeded = 3,
		Timestamp tts = 50)
	{
		auto frameCalcOwned = std::make_unique<NiceMock<MockFrameCalculator>>();
		frameCalc = frameCalcOwned.get();
		FrameConfig cfg{};
		cfg.slotCountInFrame = kSlotsPerFrame;
		cfg.slotDuration = kSlotDuration;
		ON_CALL(*frameCalc, frameConfig()).WillByDefault(Return(cfg));
		ON_CALL(*frameCalc, frameMoment(_)).WillByDefault([](Timestamp t) {
			const auto frameLen = static_cast<Timestamp>(kSlotsPerFrame) * kSlotDuration;
			FrameMoment fm{};
			fm.frameNumber = static_cast<std::uint64_t>(t / frameLen);
			fm.slotNumber = static_cast<std::uint64_t>((t % frameLen) / kSlotDuration);
			fm.timeOfSlot = static_cast<double>(t);
			return fm;
		});
		ON_CALL(*frameCalc, slotBeginTime(_, _)).WillByDefault(
			[](IFrameCalculator::FrameNum f, IFrameCalculator::SlotNum s) {
				return static_cast<Timestamp>(
					f * kSlotsPerFrame * kSlotDuration + s * kSlotDuration);
			});

		auto dfsOwned = std::make_unique<NiceMock<MockDynamicFrameSettings>>();
		dfs = dfsOwned.get();
		ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
		ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));

		StarStationInitData init{
			.sendFunc = [this](Timestamp t, std::shared_ptr<IMessage> m) {
				sent.emplace_back(t, std::move(m));
			},
			.frameCalculator = std::move(frameCalcOwned),
			.dynamicFrameSettings = std::move(dfsOwned),
			.id = id,
			.messagesNeeded = messagesNeeded,
			.tts = tts,
			.rng = rng,
		};
		return StarStationFactory::make(std::move(init));
	}
};

} // namespace

TEST(StarStation, ReportsConfiguredIdentity)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/42, /*messagesNeeded=*/3, /*tts=*/77);
	EXPECT_EQ(station->id(), 42u);
	EXPECT_EQ(station->tts(), 77);
}

TEST(StarStation, InitialStateIsAcquisitionAndJoinedTimeIsEmpty)
{
	StationFixture fix;
	auto station = fix.makeStation();
	EXPECT_EQ(station->currentState(), TerminalState::ACQUISITION);
	EXPECT_FALSE(station->joinedTime().has_value());
}

TEST(StarStation, ZeroMessagesNeededTransitionsToOperationOnFirstUpdate)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/1, /*messagesNeeded=*/0);

	station->update(/*currentTime=*/250);

	EXPECT_EQ(station->currentState(), TerminalState::OPERATION);
	ASSERT_TRUE(station->joinedTime().has_value());
	EXPECT_EQ(*station->joinedTime(), 250);
}

TEST(StarStation, AccessMessageForOtherStationDoesNotTriggerJoin)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/1, /*messagesNeeded=*/1);

	station->handleMessage(std::make_shared<StarHubAccessMessage>(/*id=*/999), 100);
	station->update(200);

	EXPECT_EQ(station->currentState(), TerminalState::ACQUISITION);
	EXPECT_FALSE(station->joinedTime().has_value());
}

TEST(StarStation, HandleHubPlanForwardsToDynamicFrameSettings)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/1, /*messagesNeeded=*/3);

	auto plan = makePlan(/*frame=*/0);
	EXPECT_CALL(*fix.dfs, handlePlan(plan)).Times(1);

	station->handleMessage(plan, 0);
}

TEST(StarStation, HandleHubPlanDoesNotCrashAndPersistsAcrossUpdates)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/1, /*messagesNeeded=*/3);

	station->handleMessage(makePlan(/*frame=*/0), 0);
	for (Timestamp t = 0; t < 1000; t += 100)
		station->update(t);

	EXPECT_EQ(station->currentState(), TerminalState::ACQUISITION);
}

TEST(StarStation, ManyUpdatesWithoutPlansAreSafe)
{
	StationFixture fix;
	auto station = fix.makeStation();
	for (Timestamp t = 0; t < 5000; t += 25)
		station->update(t);
	EXPECT_EQ(station->currentState(), TerminalState::ACQUISITION);
}

TEST(StarStation, ForwardsOperationPlanToDynamicFrameSettings)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/1, /*messagesNeeded=*/3);

	auto opPlan = std::make_shared<OperationPlanMessage>(
		/*frame=*/2,
		std::vector<OperationPlanMessage::StationAllocation>{ {1, 200} });
	EXPECT_CALL(*fix.dfs, handleOperationPlan(opPlan)).Times(1);

	station->handleMessage(opPlan, 0);
}

TEST(StarStation, OperationStateEmitsBacklogReportOnFrameTransition)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/3, /*messagesNeeded=*/0, /*tts=*/0);

	CbrTrafficProfileConfig cfg;
	cfg.bitsPerTimestamp = 1.0; // 1000 bits per 1000-tick frame.
	station->setTrafficProfile(TrafficProfileFactory::make(cfg));

	station->update(0);    // RA -> OperationState transition.
	station->update(1000); // First OperationState tick at frame 1.

	std::shared_ptr<BacklogReportMessage> report;
	for (auto& [_, m] : fix.sent)
		if (m->type() == MessageType::BacklogReport)
			report = std::static_pointer_cast<BacklogReportMessage>(m);

	ASSERT_NE(report, nullptr);
	EXPECT_EQ(report->stationID(), 3u);
	EXPECT_EQ(report->backlogBits(), 1000u);
}

TEST(StarStation, OperationPlanReducesReportedBacklog)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/3, /*messagesNeeded=*/0, /*tts=*/0);

	CbrTrafficProfileConfig cfg;
	cfg.bitsPerTimestamp = 1.0;
	station->setTrafficProfile(TrafficProfileFactory::make(cfg));

	auto plan = std::make_shared<OperationPlanMessage>(
		/*frame=*/1,
		std::vector<OperationPlanMessage::StationAllocation>{ {3, 600} });
	ON_CALL(*fix.dfs, currentOperationPlan(1)).WillByDefault(Return(plan));

	station->update(0);
	station->update(1000);

	std::shared_ptr<BacklogReportMessage> report;
	for (auto& [_, m] : fix.sent)
		if (m->type() == MessageType::BacklogReport)
			report = std::static_pointer_cast<BacklogReportMessage>(m);

	ASSERT_NE(report, nullptr);
	EXPECT_EQ(report->backlogBits(), 400u); // 1000 arrived - 600 sent.
}

TEST(StarStation, SetTrafficProfileReplacesOldProfile)
{
	StationFixture fix;
	auto station = fix.makeStation(/*id=*/3, /*messagesNeeded=*/0, /*tts=*/0);

	CbrTrafficProfileConfig fast;
	fast.bitsPerTimestamp = 2.0;
	station->setTrafficProfile(TrafficProfileFactory::make(fast));

	station->update(0);
	station->update(1000); // Frame 1 with the fast profile -> 2000 bits accrued.

	// Switch to a slow profile mid-run.
	CbrTrafficProfileConfig slow;
	slow.bitsPerTimestamp = 0.5;
	station->setTrafficProfile(TrafficProfileFactory::make(slow));

	station->update(2000); // Frame 2 with the slow profile -> 500 more bits.

	std::vector<std::shared_ptr<BacklogReportMessage>> reports;
	for (auto& [_, m] : fix.sent)
		if (m->type() == MessageType::BacklogReport)
			reports.push_back(std::static_pointer_cast<BacklogReportMessage>(m));

	ASSERT_EQ(reports.size(), 2u);
	EXPECT_EQ(reports[0]->backlogBits(), 2000u);
	EXPECT_EQ(reports[1]->backlogBits(), 2500u);
}
