#include "SimpleSaCollisionResolver.h"

#include <map>

#include "StarTopologyEmulator/Messages/CollisionReport.h"

namespace starTopologyEmulator
{

ICollisionResolver::Outcome SimpleSaCollisionResolver::resolveFrame(
	std::vector<SlotTransmission> transmissions) const
{
	std::map<Timestamp, std::vector<std::shared_ptr<IMessage>>> bySlot;
	for (auto& transmission : transmissions)
		bySlot[transmission.slotTime].push_back(std::move(transmission.msg));

	Outcome outcome;
	for (auto& [slotTime, msgs] : bySlot)
	{
		if (msgs.size() == 1)
		{
			outcome.okCount += 1;
			outcome.toHub.push_back({ slotTime, std::move(msgs[0]) });
		}
		else if (msgs.size() > 1)
		{
			outcome.lostCount += static_cast<std::uint64_t>(msgs.size());
			outcome.toHub.push_back({ slotTime, std::make_shared<CollisionReport>() });
		}
	}

	return outcome;
}

} // namespace starTopologyEmulator
