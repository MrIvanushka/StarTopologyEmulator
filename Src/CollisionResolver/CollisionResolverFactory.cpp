#include "StarTopologyEmulator/CollisionResolver/CollisionResolverFactory.h"

#include "CrdsaSicCollisionResolver.h"
#include "SimpleSaCollisionResolver.h"

namespace starTopologyEmulator
{

std::unique_ptr<ICollisionResolver> CollisionResolverFactory::makeSimpleSa()
{
	return std::make_unique<SimpleSaCollisionResolver>();
}

std::unique_ptr<ICollisionResolver> CollisionResolverFactory::makeCrdsaSic(
	std::shared_ptr<IPhysicalLink> physicalLink,
	CrdsaSicConfig config)
{
	return std::make_unique<CrdsaSicCollisionResolver>(std::move(physicalLink), config);
}

} // namespace starTopologyEmulator
