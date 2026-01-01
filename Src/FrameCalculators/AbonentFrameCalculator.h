#pragma once

#include "CommonFrameCalculator.h"

namespace starTopologyEmulator
{

class AbonentFrameCalculator : public CommonFrameCalculator
{
public:
	AbonentFrameCalculator(FrameConfig frameConfig, Timestamp tts);
private:
	virtual void correctTimeShiftImpl(Timestamp) override;

	virtual Timestamp tts() const override;

	const Timestamp _tts;

	Timestamp _ttsCorrection = 0;
};

} // namespace starTopologyEmulator
