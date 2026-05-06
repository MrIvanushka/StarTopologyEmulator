#pragma once

#include "StarTopologyEmulator/CommonTypedefs.h"

namespace starTopologyEmulator
{

struct FrameMoment
{
	std::uint64_t frameNumber;
	std::uint64_t slotNumber;
	double timeOfSlot;
};

struct FrameConfig
{
	std::uint64_t slotCountInFrame;
	Timestamp slotDuration;
	Timestamp epoch;
	std::uint64_t bitsPerSlot = 0;
};

class IFrameCalculator
{
public:
	using FrameNum = std::uint64_t;
	using SlotNum = std::uint64_t;
public:
	virtual ~IFrameCalculator() = default;

	virtual void correctTimeShift(Timestamp) = 0;

	virtual FrameMoment frameMoment(Timestamp) const = 0;

	virtual Timestamp slotBeginTime(FrameNum, SlotNum) const = 0;

	virtual const FrameConfig frameConfig() const = 0;
};

} // namespace starTopologyEmulator
