#pragma once

#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"

namespace starTopologyEmulator
{

class CommonFrameCalculator : public IFrameCalculator
{
public:
	CommonFrameCalculator(FrameConfig frameConfig);

	void correctTimeShift(Timestamp) override;

	FrameMoment frameMoment(Timestamp) const override;

	Timestamp slotBeginTime(FrameNum, SlotNum) const override;

	const FrameConfig frameConfig() const override;
private:
	virtual void correctTimeShiftImpl(Timestamp) {}

	virtual Timestamp tts() const { return 0; }

	const FrameConfig _frameConfig;
};

} // namespace starTopologyEmulator
