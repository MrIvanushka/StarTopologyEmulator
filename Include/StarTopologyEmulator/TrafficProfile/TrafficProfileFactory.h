#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"
#include "StarTopologyEmulator/TrafficProfile/BurstTrafficProfileConfig.h"
#include "StarTopologyEmulator/TrafficProfile/CbrTrafficProfileConfig.h"
#include "StarTopologyEmulator/TrafficProfile/PoissonTrafficProfileConfig.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT TrafficProfileFactory
{
public:
	static std::unique_ptr<ITrafficProfile> make(const CbrTrafficProfileConfig&);
	static std::unique_ptr<ITrafficProfile> make(const PoissonTrafficProfileConfig&);
	static std::unique_ptr<ITrafficProfile> make(const BurstTrafficProfileConfig&);
};

} // namespace starTopologyEmulator
