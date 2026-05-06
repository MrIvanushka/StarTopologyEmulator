#include <gtest/gtest.h>

#include "Helpers.h"
#include "StarTopologyEmulator/FrameCalculatorFactory.h"

using namespace starTopologyEmulator;
using namespace tests;

namespace
{

constexpr std::uint64_t kSlots = 10;
constexpr Timestamp kSlotDuration = 100;

}

TEST(AbonentFrameCalculator, FrameMomentZero)
{
	auto fc = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(kSlots, kSlotDuration), /*tts=*/0);
	const auto m = fc->frameMoment(0);
	EXPECT_EQ(m.frameNumber, 0u);
	EXPECT_EQ(m.slotNumber, 0u);
	EXPECT_DOUBLE_EQ(m.timeOfSlot, 0.0);
}

TEST(AbonentFrameCalculator, AdvancesBySlotDuration)
{
	auto fc = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(kSlots, kSlotDuration), 0);
	const auto m = fc->frameMoment(kSlotDuration);
	EXPECT_EQ(m.frameNumber, 0u);
	EXPECT_EQ(m.slotNumber, 1u);
}

TEST(AbonentFrameCalculator, AdvancesByFrame)
{
	auto fc = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(kSlots, kSlotDuration), 0);
	const auto m = fc->frameMoment(kSlots * kSlotDuration);
	EXPECT_EQ(m.frameNumber, 1u);
	EXPECT_EQ(m.slotNumber, 0u);
}

TEST(AbonentFrameCalculator, TimeOfSlotIsFractional)
{
	auto fc = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(kSlots, kSlotDuration), 0);
	const auto m = fc->frameMoment(50);
	EXPECT_EQ(m.frameNumber, 0u);
	EXPECT_EQ(m.slotNumber, 0u);
	EXPECT_NEAR(m.timeOfSlot, 0.5, 1e-9);
}

TEST(AbonentFrameCalculator, SlotBeginTimeMatchesGrid)
{
	auto fc = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(kSlots, kSlotDuration), 0);
	EXPECT_EQ(fc->slotBeginTime(0, 0), 0);
	EXPECT_EQ(fc->slotBeginTime(0, 1), kSlotDuration);
	EXPECT_EQ(fc->slotBeginTime(1, 0), static_cast<Timestamp>(kSlots * kSlotDuration));
	EXPECT_EQ(fc->slotBeginTime(2, 3), static_cast<Timestamp>(2 * kSlots * kSlotDuration + 3 * kSlotDuration));
}

TEST(AbonentFrameCalculator, TtsShiftsTime)
{
	// Abonent's tts is subtracted from absolute time before computing frame.
	// Equivalently: with tts > 0, slotBeginTime decreases relative to tts=0.
	auto noTts = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(kSlots, kSlotDuration), 0);
	auto withTts = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(kSlots, kSlotDuration), /*tts=*/50);

	EXPECT_EQ(noTts->slotBeginTime(1, 0) - withTts->slotBeginTime(1, 0), 50);
}

TEST(AbonentFrameCalculator, CorrectTimeShiftCancelsTts)
{
	// tts subtracted is _tts - _ttsCorrection, so adding correction equal to tts brings it to zero.
	auto fc = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(kSlots, kSlotDuration), /*tts=*/50);
	const auto before = fc->slotBeginTime(1, 0);
	fc->correctTimeShift(50);
	const auto after = fc->slotBeginTime(1, 0);

	EXPECT_EQ(after - before, 50);
}

TEST(AbonentFrameCalculator, FrameConfigPropagated)
{
	auto fc = FrameCalculatorFactory::abonentCalculator(makeFrameConfig(8, 250, /*epoch=*/100), 0);
	const auto cfg = fc->frameConfig();
	EXPECT_EQ(cfg.slotCountInFrame, 8u);
	EXPECT_EQ(cfg.slotDuration, 250);
	EXPECT_EQ(cfg.epoch, 100);
}

TEST(HubFrameCalculator, TtsAddsToTimeShift)
{
	// Hub: tts() returns -_tts, i.e. the inverse sign of abonent.
	auto noTts = FrameCalculatorFactory::hubCalculator(makeFrameConfig(kSlots, kSlotDuration), 0);
	auto withTts = FrameCalculatorFactory::hubCalculator(makeFrameConfig(kSlots, kSlotDuration), /*tts=*/50);

	EXPECT_EQ(withTts->slotBeginTime(1, 0) - noTts->slotBeginTime(1, 0), 50);
}

TEST(HubFrameCalculator, FrameMomentRoundTrip)
{
	auto fc = FrameCalculatorFactory::hubCalculator(makeFrameConfig(kSlots, kSlotDuration), 0);
	const Timestamp t = fc->slotBeginTime(3, 4);
	const auto m = fc->frameMoment(t);
	EXPECT_EQ(m.frameNumber, 3u);
	EXPECT_EQ(m.slotNumber, 4u);
}
