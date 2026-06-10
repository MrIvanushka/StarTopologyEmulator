#include "StarTopologyEmulator/Phy/PhysicalLinkFactory.h"

#include "StubPhysicalLink.h"

#ifdef STE_WITH_AFF3CT
#include "Aff3ctPhysicalLink.h"
#endif

namespace starTopologyEmulator
{

std::unique_ptr<IPhysicalLink> PhysicalLinkFactory::makeStub(StubPhysicalLinkConfig config)
{
	return std::make_unique<StubPhysicalLink>(config);
}

#ifdef STE_WITH_AFF3CT
std::unique_ptr<IPhysicalLink> PhysicalLinkFactory::makeAff3ctLdpc(Aff3ctPhysicalLinkConfig config)
{
	return std::make_unique<Aff3ctPhysicalLink>(std::move(config));
}
#endif

} // namespace starTopologyEmulator
