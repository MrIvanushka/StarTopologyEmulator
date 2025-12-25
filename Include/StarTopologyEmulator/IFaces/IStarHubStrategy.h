#pragma once

#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IStarHubStrategy
{
public:
	virtual ~IStarHubStrategy() = default;

	virtual StarHubPlanMessage generate(double g, double plr) = 0;
};

} // namespace starTopologyEmulator
