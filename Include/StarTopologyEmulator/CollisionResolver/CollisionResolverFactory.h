#pragma once

#include <memory>

#include "StarTopologyEmulator/CollisionResolver/CrdsaSicConfig.h"
#include "StarTopologyEmulator/IFaces/ICollisionResolver.h"
#include "StarTopologyEmulator/IFaces/IPhysicalLink.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT CollisionResolverFactory
{
public:
	static std::unique_ptr<ICollisionResolver> makeSimpleSa();

	static std::unique_ptr<ICollisionResolver> makeCrdsaSic(
		std::shared_ptr<IPhysicalLink> physicalLink,
		CrdsaSicConfig config = {});
};

} // namespace starTopologyEmulator
