#include "StarTopologyEmulator/TrafficProfile/TrafficProfileFactory.h"

#include "CbrTrafficProfile.h"

namespace starTopologyEmulator
{

std::unique_ptr<ITrafficProfile> TrafficProfileFactory::make(const CbrTrafficProfileConfig& cfg)
{
	return std::make_unique<CbrTrafficProfile>(cfg);
}

} // namespace starTopologyEmulator
