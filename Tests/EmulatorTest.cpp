#include <memory>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Helpers.h"
#include "Mocks/MockFrameCalculator.h"
#include "Mocks/MockStarHub.h"
#include "Mocks/MockStarStation.h"
#include "StarTopologyEmulator/EmulatorFactory.h"
#include "StarTopologyEmulator/FrameCalculatorFactory.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/Messages/StarStationMessage.h"
#include "StarTopologyEmulator/TerminalState.h"

using namespace starTopologyEmulator;
using namespace tests;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;

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
	NiceMock<MockFrameCalculator>* abonentFrameCalc = nullptr;
	NiceMock<MockFrameCalculator>* hubFrameCalc = nullptr;

	std::unique_ptr<IEmulator> makeEmulator(int stationCount)
	{
		EmulatorInitData init;

		init.stationFactory = [this](EmulatorInitData::SendFunc sendFunc, StationID stationID,
			std::shared_ptr<IStationStatsCollector>) {
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

		FrameConfig cfg{};
		cfg.slotCountInFrame = kSlotsPerFrame;
		cfg.slotDuration = kSlotDuration;
		cfg.epoch = 0;

		auto setupCalc = [&cfg](NiceMock<MockFrameCalculator>* fc) {
			ON_CALL(*fc, frameConfig()).WillByDefault(Return(cfg));
			ON_CALL(*fc, frameMoment(_)).WillByDefault([](Timestamp t) {
				FrameMoment fm{};
				fm.frameNumber = static_cast<std::uint64_t>(t / kFrameDuration);
				fm.slotNumber = static_cast<std::uint64_t>((t % kFrameDuration) / kSlotDuration);
				fm.timeOfSlot = static_cast<double>(t);
				return fm;
			});
			ON_CALL(*fc, slotBeginTime(_, _)).WillByDefault(
				[](IFrameCalculator::FrameNum f, IFrameCalculator::SlotNum s) {
					return static_cast<Timestamp>(f * kFrameDuration + s * kSlotDuration);
				});
		};

		auto abonentFc = std::make_unique<NiceMock<MockFrameCalculator>>();
		abonentFrameCalc = abonentFc.get();
		setupCalc(abonentFrameCalc);

		auto hubFc = std::make_unique<NiceMock<MockFrameCalculator>>();
		hubFrameCalc = hubFc.get();
		setupCalc(hubFrameCalc);

		init.abonentFrameCalculator = std::move(abonentFc);
		init.hubFrameCalculator = std::move(hubFc);
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

TEST(Emulator, MessageSentAtAbonentFrame1Slot2ArrivesAtHubFrame1Slot2)
{
	constexpr Timestamp kTts = 100;

	auto frameCfg = makeFrameConfig(kSlotsPerFrame, kSlotDuration);
	auto abonentCalc = FrameCalculatorFactory::abonentCalculator(frameCfg, kTts);
	auto hubCalc = FrameCalculatorFactory::hubCalculator(frameCfg, kTts);
	auto* abonentCalcPtr = abonentCalc.get();
	auto* hubCalcPtr = hubCalc.get();

	std::shared_ptr<NiceMock<MockStarStation>> station;
	EmulatorInitData::SendFunc stationSendFunc;
	std::shared_ptr<NiceMock<MockStarHub>> hub;

	EmulatorInitData init;
	init.stationFactory = [&](EmulatorInitData::SendFunc sendFunc, StationID stationID,
		std::shared_ptr<IStationStatsCollector>) {
		station = std::make_shared<NiceMock<MockStarStation>>();
		ON_CALL(*station, id()).WillByDefault(Return(stationID));
		ON_CALL(*station, currentState()).WillByDefault(Return(TerminalState::ACQUISITION));
		ON_CALL(*station, tts()).WillByDefault(Return(kTts));
		ON_CALL(*station, joinedTime()).WillByDefault(Return(std::nullopt));
		stationSendFunc = sendFunc;
		return station;
	};
	init.hubFactory = [&](EmulatorInitData::SendFunc) {
		hub = std::make_shared<NiceMock<MockStarHub>>();
		ON_CALL(*hub, tts()).WillByDefault(Return(kTts));
		return hub;
	};
	init.abonentFrameCalculator = std::move(abonentCalc);
	init.hubFrameCalculator = std::move(hubCalc);
	init.stationCount = 1;

	auto emu = EmulatorFactory::make(std::move(init));

	Timestamp sendTime = 0;
	for (Timestamp t = 0; t < EmulatorFixture::framesToTime(10); ++t)
	{
		const auto fm = abonentCalcPtr->frameMoment(t);
		if (fm.frameNumber == 1 && fm.slotNumber == 2)
		{
			sendTime = t;
			break;
		}
	}
	const auto sendMoment = abonentCalcPtr->frameMoment(sendTime);
	ASSERT_EQ(sendMoment.frameNumber, 1u);
	ASSERT_EQ(sendMoment.slotNumber, 2u);

	Timestamp receivedTime = -1;
	EXPECT_CALL(*hub, handleMessage(_, _)).WillOnce(SaveArg<1>(&receivedTime));

	auto msg = std::make_shared<StarStationMessage>(/*id=*/0);
	stationSendFunc(sendTime, msg);

	emu->update(EmulatorFixture::framesToTime(10));

	const auto recvMoment = hubCalcPtr->frameMoment(receivedTime);
	EXPECT_EQ(recvMoment.frameNumber, 1u);
	EXPECT_EQ(recvMoment.slotNumber, 2u);
}
