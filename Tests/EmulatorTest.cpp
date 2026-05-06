#include <memory>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Mocks/MockFrameCalculator.h"
#include "Mocks/MockStarHub.h"
#include "Mocks/MockStarStation.h"
#include "StarTopologyEmulator/EmulatorFactory.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/Messages/StarStationMessage.h"
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
constexpr Timestamp kFrameDuration =
	static_cast<Timestamp>(kSlotsPerFrame) * kSlotDuration;

struct EmulatorFixture
{
	std::vector<std::shared_ptr<NiceMock<MockStarStation>>> stations;
	std::vector<EmulatorInitData::SendFunc> stationSendFuncs;
	std::shared_ptr<NiceMock<MockStarHub>> hub;
	EmulatorInitData::SendFunc hubSendFunc;
	NiceMock<MockFrameCalculator>* frameCalc = nullptr;

	std::unique_ptr<IEmulator> makeEmulator(int stationCount)
	{
		EmulatorInitData init;

		init.stationFactory = [this](EmulatorInitData::SendFunc sendFunc, StationID stationID) {
			auto station = std::make_shared<NiceMock<MockStarStation>>();
			ON_CALL(*station, id()).WillByDefault(Return(stationID));
			ON_CALL(*station, currentState()).WillByDefault(Return(TerminalState::ACQUISITION));
			ON_CALL(*station, tts()).WillByDefault(Return(0));
			ON_CALL(*station, joinedTime()).WillByDefault(Return(std::nullopt));
			stations.push_back(station);
			stationSendFuncs.push_back(sendFunc);
			return station;
		};

		init.hubFactory = [this](EmulatorInitData::SendFunc sendFunc) {
			hub = std::make_shared<NiceMock<MockStarHub>>();
			ON_CALL(*hub, tts()).WillByDefault(Return(0));
			hubSendFunc = sendFunc;
			return hub;
		};

		auto fc = std::make_unique<NiceMock<MockFrameCalculator>>();
		frameCalc = fc.get();

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

		init.frameCalculator = std::move(fc);
		init.stationCount = stationCount;

		return EmulatorFactory::make(std::move(init));
	}

	static Timestamp framesToTime(std::uint64_t frames)
	{
		return static_cast<Timestamp>(frames) * kFrameDuration;
	}
};

} // namespace

TEST(Emulator, BuildsConfiguredNumberOfStations)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(/*stationCount=*/5);
	EXPECT_EQ(emu->stations().size(), 5u);
	EXPECT_EQ(fix.stations.size(), 5u);
}

TEST(Emulator, BuildsHub)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(2);
	EXPECT_NE(emu->hub(), nullptr);
	EXPECT_EQ(emu->hub(), fix.hub);
}

TEST(Emulator, EachStationFactoryReceivesUniqueId)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(/*stationCount=*/4);
	for (std::uint32_t i = 0; i < emu->stations().size(); ++i)
		EXPECT_EQ(emu->stations()[i]->id(), i);
}

TEST(Emulator, ExposesStationsCreatedByFactory)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(3);
	const auto& stations = emu->stations();
	ASSERT_EQ(stations.size(), fix.stations.size());
	for (std::size_t i = 0; i < stations.size(); ++i)
		EXPECT_EQ(stations[i], fix.stations[i]);
}

TEST(Emulator, UpdateAtZeroDoesNotCrash)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(2);
	emu->update(0);
	SUCCEED();
}

TEST(Emulator, UpdateWithEmptyQueueDoesNotInvokeTerminals)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(/*stationCount=*/3);

	for (const auto& station : fix.stations)
		EXPECT_CALL(*station, handleMessage(_, _)).Times(0);
	EXPECT_CALL(*fix.hub, handleMessage(_, _)).Times(0);

	for (Timestamp t = 0; t <= EmulatorFixture::framesToTime(5); t += 50)
		emu->update(t);
}

TEST(Emulator, StationMessageIsRoutedToHub)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(/*stationCount=*/2);

	EXPECT_CALL(*fix.hub, handleMessage(_, _)).Times(1);

	auto msg = std::make_shared<StarStationMessage>(/*id=*/0);
	fix.stationSendFuncs[0](/*sendTime=*/0, msg);

	emu->update(EmulatorFixture::framesToTime(2));
}

TEST(Emulator, HubBroadcastIsDeliveredToEveryStation)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(/*stationCount=*/3);

	for (const auto& station : fix.stations)
		EXPECT_CALL(*station, handleMessage(_, _)).Times(1);

	auto msg = std::make_shared<StarStationMessage>(/*id=*/42);
	fix.hubSendFunc(/*sendTime=*/0, msg);

	emu->update(EmulatorFixture::framesToTime(2));
}

TEST(Emulator, CollidingStationMessagesProduceCollisionReport)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(/*stationCount=*/2);

	// Two stations send at the same instant: the hub should receive exactly one
	// (collision) message after the queue is drained.
	EXPECT_CALL(*fix.hub, handleMessage(_, _)).Times(1);

	fix.stationSendFuncs[0](/*sendTime=*/0, std::make_shared<StarStationMessage>(0));
	fix.stationSendFuncs[1](/*sendTime=*/0, std::make_shared<StarStationMessage>(1));

	emu->update(EmulatorFixture::framesToTime(2));
}

TEST(Emulator, ZeroStationsIsValid)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(0);
	EXPECT_TRUE(emu->stations().empty());
	EXPECT_NE(emu->hub(), nullptr);
	emu->update(EmulatorFixture::framesToTime(2));
}

TEST(Emulator, LongSimulationDoesNotCrash)
{
	EmulatorFixture fix;
	auto emu = fix.makeEmulator(/*stationCount=*/4);

	const Timestamp horizon = EmulatorFixture::framesToTime(200);
	for (Timestamp t = 0; t <= horizon; t += 25)
		emu->update(t);

	for (const auto& s : emu->stations())
		EXPECT_NO_THROW(s->currentState());
}
