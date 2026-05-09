#include "StarTopologyEmulator/TrafficProfile/TrafficProfileFactory.h"

#include "BurstTrafficProfile.h"
#include "CbrTrafficProfile.h"
#include "PoissonTrafficProfile.h"

namespace starTopologyEmulator
{

std::unique_ptr<ITrafficProfile> TrafficProfileFactory::make(const CbrTrafficProfileConfig& cfg)
{
	return std::make_unique<CbrTrafficProfile>(cfg);
}

std::unique_ptr<ITrafficProfile> TrafficProfileFactory::make(const PoissonTrafficProfileConfig& cfg)
{
	return std::make_unique<PoissonTrafficProfile>(cfg);
}

std::unique_ptr<ITrafficProfile> TrafficProfileFactory::make(const BurstTrafficProfileConfig& cfg)
{
	return std::make_unique<BurstTrafficProfile>(cfg);
}

} // namespace starTopologyEmulator
