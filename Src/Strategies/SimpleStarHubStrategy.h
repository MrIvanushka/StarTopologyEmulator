#pragma once

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"

namespace starTopologyEmulator
{

class SimpleStarHubStrategy : public IStarHubStrategy
{
public:
	StarHubPlanMessage generate(double g, double plr) override;
};

} // namespace starTopologyEmulator
