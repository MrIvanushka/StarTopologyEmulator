#pragma once

#include "CommonFrameCalculator.h"

namespace starTopologyEmulator
{

class HubFrameCalculator : public CommonFrameCalculator
{
public:
	HubFrameCalculator(FrameConfig frameConfig, Timestamp tts);
private:
	virtual Timestamp tts() const override;

	const Timestamp _tts;
};

} // namespace starTopologyEmulator
