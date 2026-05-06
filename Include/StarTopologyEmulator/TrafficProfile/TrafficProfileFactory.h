#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"
#include "StarTopologyEmulator/TrafficProfile/CbrTrafficProfileConfig.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT TrafficProfileFactory
{
public:
	static std::unique_ptr<ITrafficProfile> make(const CbrTrafficProfileConfig&);
};

} // namespace starTopologyEmulator
