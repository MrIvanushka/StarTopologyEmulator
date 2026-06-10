#pragma once

#include <memory>

#include "StarTopologyEmulator/CollisionResolver/CrdsaFrameModel.h"
#include "StarTopologyEmulator/CollisionResolver/CrdsaSicConfig.h"
#include "StarTopologyEmulator/IFaces/ICollisionResolver.h"
#include "StarTopologyEmulator/IFaces/IPhysicalLink.h"

namespace starTopologyEmulator
{

class CrdsaSicCollisionResolver : public ICollisionResolver
{
public:
	CrdsaSicCollisionResolver(std::shared_ptr<IPhysicalLink> physicalLink, CrdsaSicConfig config);

	Outcome resolveFrame(std::vector<SlotTransmission> transmissions) const override;

private:
	mutable CrdsaFrameModel _model;
};

} // namespace starTopologyEmulator
