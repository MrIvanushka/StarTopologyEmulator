#include "CrdsaSicCollisionResolver.h"

namespace starTopologyEmulator
{

CrdsaSicCollisionResolver::CrdsaSicCollisionResolver(
	std::shared_ptr<IPhysicalLink> physicalLink,
	CrdsaSicConfig config)
	: _model(std::move(physicalLink), config)
{ }

ICollisionResolver::Outcome CrdsaSicCollisionResolver::resolveFrame(
	std::vector<SlotTransmission> transmissions) const
{
	Outcome outcome;
	if (transmissions.empty())
		return outcome;

	const auto frame = _model.process(transmissions.size());

	for (std::size_t i = 0; i < transmissions.size(); ++i)
	{
		if (i < frame.recovered.size() && frame.recovered[i])
			outcome.toHub.push_back({ transmissions[i].slotTime, std::move(transmissions[i].msg) });
	}

	outcome.okCount = frame.recoveredCount;
	outcome.lostCount = transmissions.size() - frame.recoveredCount;

	return outcome;
}

} // namespace starTopologyEmulator
