#pragma once

#include <random>

#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/TrafficProfile/PoissonTrafficProfileConfig.h"

namespace starTopologyEmulator
{

class PoissonTrafficProfile : public ITrafficProfile
{
public:
	explicit PoissonTrafficProfile(PoissonTrafficProfileConfig);

	std::uint64_t generateBits(Timestamp duration) override;

private:
	PoissonTrafficProfileConfig _cfg;
	std::mt19937 _rng;
};

} // namespace starTopologyEmulator
