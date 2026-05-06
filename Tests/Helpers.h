#pragma once

#include <cstdint>
#include <memory>

#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace tests
{

using namespace starTopologyEmulator;

inline FrameConfig makeFrameConfig(
	std::uint64_t slotCountInFrame = 10,
	Timestamp slotDuration = 100,
	Timestamp epoch = 0,
	std::uint64_t bitsPerSlot = 0)
{
	FrameConfig fc{};
	fc.slotCountInFrame = slotCountInFrame;
	fc.slotDuration = slotDuration;
	fc.epoch = epoch;
	fc.bitsPerSlot = bitsPerSlot;
	return fc;
}

inline std::shared_ptr<StarHubPlanMessage> makePlan(
	std::uint64_t frame,
	std::uint8_t raSlots = 5,
	double pTx = 1.0,
	std::uint8_t baseWindow = 1)
{
	StarHubPlanMessage::FtpConfig ftp;
	ftp.randomAccessSlotsCountInFrame = raSlots;
	ftp.yellowSlotsCountInFrame = 0;
	ftp.onlineSlotsCountInFrame = 0;

	StarHubPlanMessage::BackoffConfig backoff;
	backoff.pTx = pTx;
	backoff.baseWindow = baseWindow;
	backoff.maxWindow = 32;
	backoff.useExponential = false;
	backoff.exponentBase = 2.0;

	return std::make_shared<StarHubPlanMessage>(frame, ftp, backoff);
}

} // namespace tests
