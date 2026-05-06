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
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"
#include "StarTopologyEmulator/Stations/StarStationFactory.h"
#include "StarTopologyEmulator/TerminalState.h"

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
