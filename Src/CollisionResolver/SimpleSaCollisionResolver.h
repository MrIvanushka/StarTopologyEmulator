#pragma once

#include "StarTopologyEmulator/IFaces/ICollisionResolver.h"

namespace starTopologyEmulator
{

class SimpleSaCollisionResolver : public ICollisionResolver
{
public:
	Outcome resolveFrame(std::vector<SlotTransmission> transmissions) const override;
};

} // namespace starTopologyEmulator
