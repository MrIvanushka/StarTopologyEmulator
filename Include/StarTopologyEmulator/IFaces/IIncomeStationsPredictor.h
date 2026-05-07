#pragma once

#include <cstdint>

namespace starTopologyEmulator
{

class IIncomeStationsPredictor
{
public:
	virtual ~IIncomeStationsPredictor() = default;

	virtual double estimateReadyUsers(std::uint64_t currentFrame, std::uint64_t targetFrame) = 0;
};

} // namespace starTopologyEmulator
