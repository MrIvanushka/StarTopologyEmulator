#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IMetricProducer.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IStarHubStrategy : public IMetricProducer
{
public:
	virtual std::shared_ptr<StarHubPlanMessage> generate(std::uint64_t frame, double g, double plr) = 0;
};

} // namespace starTopologyEmulator
