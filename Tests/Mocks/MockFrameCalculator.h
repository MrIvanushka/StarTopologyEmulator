#pragma once

#include <gmock/gmock.h>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"

namespace tests
{

class MockFrameCalculator : public starTopologyEmulator::IFrameCalculator
{
public:
	MOCK_METHOD(void, correctTimeShift, (starTopologyEmulator::Timestamp), (override));
	MOCK_METHOD(starTopologyEmulator::FrameMoment, frameMoment,
		(starTopologyEmulator::Timestamp), (const, override));
	MOCK_METHOD(starTopologyEmulator::Timestamp, slotBeginTime,
		(FrameNum, SlotNum), (const, override));
	MOCK_METHOD((const starTopologyEmulator::FrameConfig), frameConfig, (), (const, override));
};

} // namespace tests
