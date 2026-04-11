#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IMetricProducer.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace starTopologyEmulator
{

class IFtpGenerator : public IMetricProducer
{
public:
	virtual StarHubPlanMessage::FtpConfig generate(std::uint64_t frame) = 0;
};

} // namespace starTopologyEmulator
