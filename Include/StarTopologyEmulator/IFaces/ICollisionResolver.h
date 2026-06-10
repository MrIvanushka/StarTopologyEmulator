#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IMessage.h"

namespace starTopologyEmulator
{

class ICollisionResolver
{
public:
	struct SlotTransmission
	{
		Timestamp slotTime;
		std::uint64_t slotNumber;
		std::shared_ptr<IMessage> msg;
	};

	struct DeliveredMessage
	{
		Timestamp deliveryTime;
		std::shared_ptr<IMessage> msg;
	};

	struct Outcome
	{
		std::vector<DeliveredMessage> toHub;
		std::uint64_t okCount = 0;
		std::uint64_t lostCount = 0;
	};

	virtual ~ICollisionResolver() = default;

	virtual Outcome resolveFrame(std::vector<SlotTransmission> transmissions) const = 0;
};

} // namespace starTopologyEmulator
