#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Helpers.h"
#include "Mocks/MockBacklogAccumulator.h"
#include "Mocks/MockDynamicFrameSettings.h"
#include "Mocks/MockFrameCalculator.h"
#include "Mocks/MockIncomeLoadEstimator.h"
#include "Mocks/MockStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/Messages/BacklogReportMessage.h"
#include "StarTopologyEmulator/Messages/OperationPlanMessage.h"
#include "StarTopologyEmulator/Messages/StarHubAccessMessage.h"
#include "StarTopologyEmulator/Messages/StarStationMessage.h"
#include "StarTopologyEmulator/Stations/StarHubFactory.h"

using namespace starTopologyEmulator;
using namespace tests;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace
{

constexpr std::uint64_t kSlotsPerFrame = 10;
constexpr Timestamp kSlotDuration = 100;
constexpr Timestamp kFrameDuration =
	static_cast<Timestamp>(kSlotsPerFrame) * kSlotDuration;

struct HubFixture
{
	std::vector<std::pair<Timestamp, std::shared_ptr<IMessage>>> sent;
	std::shared_ptr<NiceMock<MockIncomeLoadEstimator>> estimator;
	std::shared_ptr<NiceMock<MockFrameCalculator>> frameCalc;
	std::shared_ptr<NiceMock<MockDynamicFrameSettings>> dfs;
	NiceMock<MockStarHubStrategy>* strategy = nullptr;
	NiceMock<MockBacklogAccumulator>* backlogAccumulator = nullptr;

	std::shared_ptr<IStarHub> makeHub(Timestamp tts = 0)
	{
		estimator = std::make_shared<NiceMock<MockIncomeLoadEstimator>>();
		ON_CALL(*estimator, incomeLoad()).WillByDefault(Return(0.0));
		ON_CALL(*estimator, plr()).WillByDefault(Return(0.0));

		frameCalc = std::make_shared<NiceMock<MockFrameCalculator>>();
		FrameConfig cfg{};
		cfg.slotCountInFrame = kSlotsPerFrame;
		cfg.slotDuration = kSlotDuration;
		cfg.epoch = 0;
		ON_CALL(*frameCalc, frameConfig()).WillByDefault(Return(cfg));
		ON_CALL(*frameCalc, frameMoment(_)).WillByDefault([](Timestamp t) {
			FrameMoment fm{};
			fm.frameNumber = static_cast<std::uint64_t>(t / kFrameDuration);
			fm.slotNumber = static_cast<std::uint64_t>((t % kFrameDuration) / kSlotDuration);
			fm.timeOfSlot = static_cast<double>(t);
			return fm;
		});
		ON_CALL(*frameCalc, slotBeginTime(_, _)).WillByDefault(
			[](IFrameCalculator::FrameNum f, IFrameCalculator::SlotNum s) {
				return static_cast<Timestamp>(f * kFrameDuration + s * kSlotDuration);
			});

		dfs = std::make_shared<NiceMock<MockDynamicFrameSettings>>();
		ON_CALL(*dfs, currentPlan(_)).WillByDefault(Return(nullptr));
		ON_CALL(*dfs, earliestPlanNumber()).WillByDefault(Return(0u));

		auto strategyOwned = std::make_unique<NiceMock<MockStarHubStrategy>>();
		strategy = strategyOwned.get();
		ON_CALL(*strategy, generate(_, _)).WillByDefault(
			[](std::uint64_t /*current*/, std::uint64_t target) {
				return makePlan(target);
			});

		auto accOwned = std::make_unique<NiceMock<MockBacklogAccumulator>>();
		backlogAccumulator = accOwned.get();
		ON_CALL(*backlogAccumulator, generateOperationPlan(_, _)).WillByDefault(
			[](std::uint64_t frame, std::uint64_t /*slots*/) {
				return std::make_shared<OperationPlanMessage>(
					frame,
					std::vector<OperationPlanMessage::StationAllocation>{});
			});

		StarHubInitData init{
			.sendFunc = [this](Timestamp t, std::shared_ptr<IMessage> m) {
				sent.emplace_back(t, std::move(m));
			},
			.incomeLoadEstimator = estimator,
			.frameCalculator = frameCalc,
			.dynamicFrameSettings = dfs,
			.strategy = std::move(strategyOwned),
			.backlogAccumulator = std::move(accOwned),
			.tts = tts,
		};
		return StarHubFactory::make(std::move(init));
	}

	static Timestamp framesToTime(std::uint64_t frames)
	{
		return static_cast<Timestamp>(frames) * kFrameDuration;
	}
};

} // namespace

TEST(StarHub, ReportsConfiguredTts)
{
	HubFixture fix;
	auto hub = fix.makeHub(/*tts=*/123);
	EXPECT_EQ(hub->tts(), 123);
}

TEST(StarHub, UpdateBeforeFrameBoundaryEmitsNothing)
{
	HubFixture fix;
	auto hub = fix.makeHub();

	EXPECT_CALL(*fix.dfs, handlePlan(_)).Times(0);
	EXPECT_CALL(*fix.strategy, generate(_, _)).Times(0);

	hub->update(0);
	EXPECT_TRUE(fix.sent.empty());
}

TEST(StarHub, UpdatePastFrameBoundaryEmitsAndStoresPlan)
{
	HubFixture fix;
	auto hub = fix.makeHub();

	EXPECT_CALL(*fix.strategy, generate(0, 2))
		.Times(1)
		.WillOnce(Return(makePlan(/*frame=*/2)));
	EXPECT_CALL(*fix.dfs, handlePlan(_)).Times(1);

	hub->update(HubFixture::framesToTime(1));     // crosses frame 0 -> 1, triggers onFrameEnd(0)

	ASSERT_FALSE(fix.sent.empty());
	const auto& [_, msg] = fix.sent.front();
	ASSERT_NE(msg, nullptr);
	EXPECT_EQ(msg->type(), MessageType::StarHubPlan);
}

TEST(StarHub, HandleMessageWithoutPlanIsNoop)
{
	HubFixture fix;
	auto hub = fix.makeHub();

	hub->handleMessage(std::make_shared<StarStationMessage>(/*id=*/3), 0);
	EXPECT_TRUE(fix.sent.empty());
}

TEST(StarHub, SuccessfulMessageQueuesAnswerOnFrameEnd)
{
	HubFixture fix;
	auto hub = fix.makeHub();

	auto seededPlan = makePlan(/*frame=*/0, /*ra=*/5, /*pTx=*/1.0, /*baseWindow=*/1);
	ON_CALL(*fix.dfs, currentPlan(0u)).WillByDefault(Return(seededPlan));

	hub->handleMessage(std::make_shared<StarStationMessage>(/*id=*/9), /*arrivalTime=*/0);
	hub->update(HubFixture::framesToTime(1));     // triggers onFrameEnd(0) and answer broadcast.

	bool foundAccessForId9 = false;
	for (const auto& [t, m] : fix.sent)
	{
		if (m->type() != MessageType::StarHubAccess)
			continue;
		auto access = std::static_pointer_cast<StarHubAccessMessage>(m);
		if (access->stationID() == 9u)
			foundAccessForId9 = true;
	}
	EXPECT_TRUE(foundAccessForId9);
}

TEST(StarHub, MultipleFrameUpdatesProduceMultiplePlans)
{
	HubFixture fix;
	auto hub = fix.makeHub();

	hub->update(HubFixture::framesToTime(1));
	hub->update(HubFixture::framesToTime(2));
	hub->update(HubFixture::framesToTime(3));

	std::size_t planCount = 0;
	for (const auto& [t, m] : fix.sent)
		if (m->type() == MessageType::StarHubPlan)
			++planCount;

	EXPECT_GE(planCount, 3u);
}

TEST(StarHub, BacklogReportIsForwardedToAccumulator)
{
	HubFixture fix;
	auto hub = fix.makeHub();

	auto report = std::make_shared<BacklogReportMessage>(/*id=*/5, /*bits=*/777);
	EXPECT_CALL(*fix.backlogAccumulator, handleReport(report)).Times(1);

	hub->handleMessage(report, /*arrivalTime=*/0);
}

TEST(StarHub, OnFrameEndQueriesAccumulatorWithTargetFrameAndOnlineSlots)
{
	HubFixture fix;
	auto hub = fix.makeHub();

	auto strategyPlan = std::make_shared<StarHubPlanMessage>(
		/*frame=*/2,
		StarHubPlanMessage::FtpConfig{ /*online=*/12, /*yellow=*/0, /*ra=*/5 },
		StarHubPlanMessage::BackoffConfig{});
	ON_CALL(*fix.strategy, generate(_, _)).WillByDefault(Return(strategyPlan));

	EXPECT_CALL(*fix.backlogAccumulator, generateOperationPlan(/*frame=*/2u, /*slots=*/12u))
		.Times(1)
		.WillOnce(Return(std::make_shared<OperationPlanMessage>(
			2,
			std::vector<OperationPlanMessage::StationAllocation>{})));

	hub->update(HubFixture::framesToTime(1));
}

TEST(StarHub, OperationPlanIsBroadcastAlongsideHubPlan)
{
	HubFixture fix;
	auto hub = fix.makeHub();

	hub->update(HubFixture::framesToTime(1));

	bool foundOpPlan = false;
	for (const auto& [t, m] : fix.sent)
		if (m->type() == MessageType::OperationPlan)
			foundOpPlan = true;

	EXPECT_TRUE(foundOpPlan);
}
