#pragma once

#include <random>

#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/TrafficProfile/BurstTrafficProfileConfig.h"

namespace starTopologyEmulator
{

class BurstTrafficProfile : public ITrafficProfile
{
public:
	explicit BurstTrafficProfile(BurstTrafficProfileConfig);

	std::uint64_t generateBits(Timestamp duration) override;

private:
	enum class Phase
	{
		On,
		Off
	};

	double sampleResidual(Phase);

	BurstTrafficProfileConfig _cfg;
	std::mt19937 _rng;
	Phase _phase = Phase::On;
	double _remainingInPhase = 0.0;
	bool _valid = false;
};

} // namespace starTopologyEmulator
