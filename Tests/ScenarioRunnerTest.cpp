#include <memory>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Mocks/MockEmulator.h"
#include "Mocks/MockScenarioEvent.h"
#include "Mocks/MockStarStation.h"
#include "StarTopologyEmulator/IFaces/IScenarioEvent.h"
#include "StarTopologyEmulator/Scenario/ScenarioEventFactory.h"
#include "StarTopologyEmulator/Scenario/ScenarioRunnerFactory.h"

using namespace starTopologyEmulator;
using namespace tests;
using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrictMock;

namespace
{

const std::vector<std::shared_ptr<IStarStation>> kNoStations{};

std::unique_ptr<NiceMock<MockScenarioEvent>> makeEvent(Timestamp at)
{
	auto event = std::make_unique<NiceMock<MockScenarioEvent>>();
	ON_CALL(*event, at()).WillByDefault(Return(at));
	ON_CALL(*event, kind()).WillByDefault(Return(ScenarioEventKind::STATION_ENABLE));
	return event;
}

} // namespace

// ---------------------------------------------------------------------------
// ScenarioRunner — load / prepare
// ---------------------------------------------------------------------------

TEST(ScenarioRunnerLoad, DefaultStateHasNoEvents)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	EXPECT_EQ(runner->totalEvents(), 0u);
	EXPECT_EQ(runner->consumedEvents(), 0u);
	EXPECT_TRUE(runner->finished());
	EXPECT_FALSE(runner->nextEventAt().has_value());
}

TEST(ScenarioRunnerLoad, LoadSortsEventsByTimestamp)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(makeEvent(300));
	events.push_back(makeEvent(100));
	events.push_back(makeEvent(200));
	runner->load(std::move(events));

	EXPECT_EQ(runner->nextEventAt(), Timestamp{100});
}

TEST(ScenarioRunnerLoad, PrepareAcceptsNonNegativeTimestamps)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(makeEvent(0));
	events.push_back(makeEvent(500));
	runner->load(std::move(events));

	EXPECT_TRUE(runner->prepare().empty());
}

TEST(ScenarioRunnerLoad, PrepareRejectsNegativeTimestamp)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(makeEvent(-1));
	events.push_back(makeEvent(100));
	runner->load(std::move(events));

	const auto errors = runner->prepare();
	ASSERT_EQ(errors.size(), 1u);
	EXPECT_EQ(errors[0].at, Timestamp{-1});
}

// ---------------------------------------------------------------------------
// ScenarioRunner — update / ordering
// ---------------------------------------------------------------------------

TEST(ScenarioRunnerUpdate, EventAtExactTimestampFires)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	auto eventOwned = makeEvent(100);
	auto* event = eventOwned.get();

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(std::move(eventOwned));
	runner->load(std::move(events));

	EXPECT_CALL(*event, apply(_));
	runner->update(100);
}

TEST(ScenarioRunnerUpdate, EventInFutureDoesNotFire)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	auto eventOwned = makeEvent(200);
	auto* event = eventOwned.get();

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(std::move(eventOwned));
	runner->load(std::move(events));

	EXPECT_CALL(*event, apply(_)).Times(0);
	runner->update(100);
}

TEST(ScenarioRunnerUpdate, EventAppliedBeforeEmulatorTick)
{
	auto mockEmulator = std::make_shared<StrictMock<MockEmulator>>();
	ON_CALL(*mockEmulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(mockEmulator);

	auto eventOwned = makeEvent(100);
	auto* event = eventOwned.get();

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(std::move(eventOwned));
	runner->load(std::move(events));

	InSequence seq;
	EXPECT_CALL(*event, apply(Ref(*mockEmulator)));
	EXPECT_CALL(*mockEmulator, update(IEmulator::Timestamp{100}));

	runner->update(100);
}

TEST(ScenarioRunnerUpdate, MultipleEventsAtSameTimestampAllFire)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	auto e1Owned = makeEvent(100);
	auto e2Owned = makeEvent(100);
	auto* e1 = e1Owned.get();
	auto* e2 = e2Owned.get();
	events.push_back(std::move(e1Owned));
	events.push_back(std::move(e2Owned));
	runner->load(std::move(events));

	EXPECT_CALL(*e1, apply(_));
	EXPECT_CALL(*e2, apply(_));
	runner->update(100);
}

TEST(ScenarioRunnerUpdate, EmulatorUpdateCalledEvenWithNoEvents)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	auto runner = ScenarioRunnerFactory::make(emulator);

	EXPECT_CALL(*emulator, update(IEmulator::Timestamp{50}));
	runner->update(50);
}

// ---------------------------------------------------------------------------
// ScenarioRunner — progress tracking
// ---------------------------------------------------------------------------

TEST(ScenarioRunnerProgress, ConsumedCountIncreasesAfterApply)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(makeEvent(100));
	events.push_back(makeEvent(200));
	runner->load(std::move(events));

	EXPECT_EQ(runner->consumedEvents(), 0u);
	runner->update(100);
	EXPECT_EQ(runner->consumedEvents(), 1u);
	runner->update(200);
	EXPECT_EQ(runner->consumedEvents(), 2u);
}

TEST(ScenarioRunnerProgress, FinishedAfterAllEventsConsumed)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(makeEvent(100));
	runner->load(std::move(events));

	EXPECT_FALSE(runner->finished());
	runner->update(100);
	EXPECT_TRUE(runner->finished());
}

TEST(ScenarioRunnerProgress, NextEventAtAdvancesAfterConsume)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(makeEvent(100));
	events.push_back(makeEvent(200));
	runner->load(std::move(events));

	EXPECT_EQ(runner->nextEventAt(), Timestamp{100});
	runner->update(100);
	EXPECT_EQ(runner->nextEventAt(), Timestamp{200});
	runner->update(200);
	EXPECT_FALSE(runner->nextEventAt().has_value());
}

// ---------------------------------------------------------------------------
// ScenarioRunner — reset
// ---------------------------------------------------------------------------

TEST(ScenarioRunnerReset, ResetRestoresCursorAndRefiresEvents)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	auto eventOwned = makeEvent(100);
	auto* event = eventOwned.get();

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(std::move(eventOwned));
	runner->load(std::move(events));

	EXPECT_CALL(*event, apply(_)).Times(2);
	runner->update(100);
	runner->reset();
	runner->update(100);
}

// ---------------------------------------------------------------------------
// ScenarioRunner — callback
// ---------------------------------------------------------------------------

TEST(ScenarioRunnerCallback, FiredForEachAppliedEvent)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(makeEvent(10));
	events.push_back(makeEvent(20));
	runner->load(std::move(events));

	int callbackCount = 0;
	runner->setEventAppliedCallback(
		[&](Timestamp, const IScenarioEvent&) { ++callbackCount; });

	runner->update(20);
	EXPECT_EQ(callbackCount, 2);
}

TEST(ScenarioRunnerCallback, NotFiredForFutureEvents)
{
	auto emulator = std::make_shared<NiceMock<MockEmulator>>();
	ON_CALL(*emulator, stations()).WillByDefault(ReturnRef(kNoStations));
	auto runner = ScenarioRunnerFactory::make(emulator);

	std::vector<std::unique_ptr<IScenarioEvent>> events;
	events.push_back(makeEvent(500));
	runner->load(std::move(events));

	int callbackCount = 0;
	runner->setEventAppliedCallback(
		[&](Timestamp, const IScenarioEvent&) { ++callbackCount; });

	runner->update(100);
	EXPECT_EQ(callbackCount, 0);
}

// ---------------------------------------------------------------------------
// Concrete events — station range filtering
// ---------------------------------------------------------------------------

class StationEventFixture : public testing::Test
{
protected:
	void SetUp() override
	{
		auto s2 = std::make_shared<NiceMock<MockStarStation>>();
		ON_CALL(*s2, id()).WillByDefault(Return(StationID{2}));

		auto s5 = std::make_shared<NiceMock<MockStarStation>>();
		ON_CALL(*s5, id()).WillByDefault(Return(StationID{5}));

		auto s9 = std::make_shared<NiceMock<MockStarStation>>();
		ON_CALL(*s9, id()).WillByDefault(Return(StationID{9}));

		mockStation2 = s2.get();
		mockStation5 = s5.get();
		mockStation9 = s9.get();

		stationVec = { s2, s5, s9 };

		mockEmulator = std::make_shared<NiceMock<MockEmulator>>();
		ON_CALL(*mockEmulator, stations()).WillByDefault(ReturnRef(stationVec));
	}

	std::shared_ptr<NiceMock<MockEmulator>> mockEmulator;
	std::vector<std::shared_ptr<IStarStation>> stationVec;

	MockStarStation* mockStation2 = nullptr;
	MockStarStation* mockStation5 = nullptr;
	MockStarStation* mockStation9 = nullptr;
};

TEST_F(StationEventFixture, StationEnableEnablesStationsInRange)
{
	auto event = ScenarioEventFactory::makeStationEnable(0, 2, 5);

	EXPECT_CALL(*mockStation2, setEnabled(true));
	EXPECT_CALL(*mockStation5, setEnabled(true));
	EXPECT_CALL(*mockStation9, setEnabled(_)).Times(0);

	event->apply(*mockEmulator);
}

TEST_F(StationEventFixture, StationDisableDisablesStationsInRange)
{
	auto event = ScenarioEventFactory::makeStationDisable(0, 5, 9);

	EXPECT_CALL(*mockStation2, setEnabled(_)).Times(0);
	EXPECT_CALL(*mockStation5, setEnabled(false));
	EXPECT_CALL(*mockStation9, setEnabled(false));

	event->apply(*mockEmulator);
}

TEST_F(StationEventFixture, StationSetProfileCallsBuilderPerStation)
{
	int builderCalls = 0;
	auto builder = [&]() -> std::unique_ptr<ITrafficProfile> {
		++builderCalls;
		return nullptr;
	};

	// Stations 2 and 5 are in range [2, 5]; station 9 is not.
	auto event = ScenarioEventFactory::makeStationSetProfile(0, 2, 5, builder);
	event->apply(*mockEmulator);

	EXPECT_EQ(builderCalls, 2);
}
