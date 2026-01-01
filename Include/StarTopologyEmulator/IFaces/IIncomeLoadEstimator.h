#pragma once

#include <cstdint>

#include "StarTopologyEmulator/IFaces/IMetricProducer.h"

namespace starTopologyEmulator
{

struct RandomAccessFrameResult
{
	std::uint32_t totalRaSlots = 0;

	std::uint32_t successSlots = 0;
	std::uint32_t collisionSlots = 0;
	std::uint32_t idleSlots = 0;
};

class IIncomeLoadEstimator : public IMetricProducer
{
public:
	virtual void update(const RandomAccessFrameResult& result) = 0;

	virtual double incomeLoad() const = 0;

	virtual double plr() const = 0;

	virtual void reset() = 0;
};

} // namespace starTopologyEmulator
