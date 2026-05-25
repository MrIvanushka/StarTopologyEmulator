#include "CommonFrameCalculator.h"

#include <utility>

namespace starTopologyEmulator
{

CommonFrameCalculator::CommonFrameCalculator(FrameConfig frameConfig)
	: _frameConfig(std::move(frameConfig))
{}

void CommonFrameCalculator::correctTimeShift(Timestamp howMuch)
{
	correctTimeShiftImpl(howMuch);
}

FrameMoment CommonFrameCalculator::frameMoment(Timestamp time) const
{
	time -= _frameConfig.epoch - tts();
	time = time > 0 ? time : 0;
	auto totalSlotCount = time / _frameConfig.slotDuration;
	auto timeAfterSlotBegin = time % _frameConfig.slotDuration;

	return FrameMoment(
		{
			.frameNumber = totalSlotCount / _frameConfig.slotCountInFrame,
			.slotNumber = totalSlotCount % _frameConfig.slotCountInFrame,
			.timeOfSlot = static_cast<double>(timeAfterSlotBegin) / _frameConfig.slotDuration
		});
}

Timestamp CommonFrameCalculator::slotBeginTime(FrameNum frameNum, SlotNum slotNum) const
{
	auto totalSlotCount = _frameConfig.slotCountInFrame * frameNum + slotNum;
	auto totalSlotDuration = totalSlotCount * _frameConfig.slotDuration;

	return _frameConfig.epoch - tts() + totalSlotDuration;
}

const FrameConfig CommonFrameCalculator::frameConfig() const
{
	return _frameConfig;
}

} // namespace starTopologyEmulator
