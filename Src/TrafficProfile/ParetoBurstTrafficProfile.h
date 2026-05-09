#pragma once

#include <random>

#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/TrafficProfile/ParetoBurstTrafficProfileConfig.h"

namespace starTopologyEmulator
{

class ParetoBurstTrafficProfile : public ITrafficProfile
{
public:
	explicit ParetoBurstTrafficProfile(ParetoBurstTrafficProfileConfig);

	std::uint64_t generateBits(Timestamp duration) override;

private:
	enum class Phase
	{
		On,
		Off
	};

	double sampleParetoDuration(double xm);
	double sampleResidual(Phase);

	ParetoBurstTrafficProfileConfig _cfg;
	std::mt19937 _rng;
	Phase _phase = Phase::On;
	double _remainingInPhase = 0.0;
	bool _valid = false;
};

} // namespace starTopologyEmulator
