#include "HubFrameCalculator.h"

#include <utility>

namespace starTopologyEmulator
{

HubFrameCalculator::HubFrameCalculator(FrameConfig frameConfig, Timestamp tts)
	: CommonFrameCalculator(std::move(frameConfig))
	, _tts(tts)
{}

Timestamp HubFrameCalculator::tts() const
{
	return -_tts;
}

} // namespace starTopologyEmulator