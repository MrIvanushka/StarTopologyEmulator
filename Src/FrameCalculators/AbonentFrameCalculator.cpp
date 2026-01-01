#include "AbonentFrameCalculator.h"

#include <utility>

namespace starTopologyEmulator
{

AbonentFrameCalculator::AbonentFrameCalculator(FrameConfig frameConfig, Timestamp tts)
	: CommonFrameCalculator(std::move(frameConfig))
	, _tts(tts)
{}

void AbonentFrameCalculator::correctTimeShiftImpl(Timestamp howMuch)
{
	_ttsCorrection += howMuch;
}

Timestamp AbonentFrameCalculator::tts() const
{
	return _tts - _ttsCorrection;
}

} // namespace starTopologyEmulator
