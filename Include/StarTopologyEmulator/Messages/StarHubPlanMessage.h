#pragma once

#include <cstdint>

#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubPlanMessage : public IMessage
{
public:
	enum class BackoffType
	{
		NONE,
		BEB,
		MILD,
		LMILD
	};

	struct BackoffConfig
	{
		std::uint8_t baseWindow = 1;
		std::uint8_t maxWindow = 32;
		double exponentBase = 2.0;
		BackoffType backoffType = BackoffType::BEB;
		double pTx = 1.0;
		std::uint8_t additiveStep = 1;
	};

	struct FtpConfig
	{
		std::uint8_t onlineSlotsCountInFrame = 0;
		std::uint8_t yellowSlotsCountInFrame = 0;
		std::uint8_t randomAccessSlotsCountInFrame = 0;
	};
public:
	StarHubPlanMessage(
		std::uint64_t frame,
		const FtpConfig& ftp,
		const BackoffConfig& backoff)
		: _frame(frame)
		, _ftp(ftp)
		, _backoff(backoff)
	{}

	MessageType type() const override { return MessageType::StarHubPlan; }

	std::uint64_t frame() const { return _frame; }

	std::uint8_t onlineSlotsCountInFrame() const { return _ftp.onlineSlotsCountInFrame; }
	std::uint8_t yellowSlotsCountInFrame() const { return _ftp.yellowSlotsCountInFrame; }
	std::uint8_t randomAccessSlotsCountInFrame() const { return _ftp.randomAccessSlotsCountInFrame; }
	const BackoffConfig& backoff() const { return _backoff; }

	std::uint32_t collidedStationCount() const { return _collidedStationCount; }
	void setCollidedStationCount(std::uint32_t v) { _collidedStationCount = v; }
private:
	std::uint64_t _frame;
	FtpConfig _ftp;
	BackoffConfig _backoff;
	std::uint32_t _collidedStationCount = 0;
};

} // namespace starTopologyEmulator
