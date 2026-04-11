#pragma once

#include <cstdint>

#include "StarTopologyEmulator/IFaces/IMetricProducer.h"

namespace starTopologyEmulator
{

class IIncomeStationsPredictor : public IMetricProducer
{
public:
	virtual ~IIncomeStationsPredictor() = default;

	virtual double estimateReadyUsers(std::uint64_t currentFrame, std::uint64_t targetFrame) = 0;
};

} // namespace starTopologyEmulator
