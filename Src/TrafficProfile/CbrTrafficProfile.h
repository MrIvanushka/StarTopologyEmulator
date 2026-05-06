#pragma once

#include "StarTopologyEmulator/IFaces/ITrafficProfile.h"
#include "StarTopologyEmulator/TrafficProfile/CbrTrafficProfileConfig.h"

namespace starTopologyEmulator
{

class CbrTrafficProfile : public ITrafficProfile
{
public:
	explicit CbrTrafficProfile(CbrTrafficProfileConfig);

	std::uint64_t generateBits(Timestamp duration) override;

private:
	CbrTrafficProfileConfig _cfg;
	double _residual = 0.0;
};

} // namespace starTopologyEmulator
