#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IPhysicalLink.h"
#include "StarTopologyEmulator/Phy/Aff3ctPhysicalLinkConfig.h"
#include "StarTopologyEmulator/Phy/StubPhysicalLinkConfig.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT PhysicalLinkFactory
{
public:
	static std::unique_ptr<IPhysicalLink> makeStub(StubPhysicalLinkConfig = {});

#ifdef STE_WITH_AFF3CT
	static std::unique_ptr<IPhysicalLink> makeAff3ctLdpc(Aff3ctPhysicalLinkConfig config);
#endif
};

} // namespace starTopologyEmulator
