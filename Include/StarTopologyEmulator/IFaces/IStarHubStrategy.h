#pragma once

#include "StarTopologyEmulator/IFaces/IMetricProducer.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IStarHubStrategy : public IMetricProducer
{
public:
	virtual StarHubPlanMessage generate(double g, double plr) = 0;
};

} // namespace starTopologyEmulator
