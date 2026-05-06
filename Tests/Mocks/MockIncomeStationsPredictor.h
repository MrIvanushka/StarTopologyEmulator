#pragma once

#include <cstdint>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"

namespace tests
{

class MockIncomeStationsPredictor : public starTopologyEmulator::IIncomeStationsPredictor
{
public:
	MOCK_METHOD(double, estimateReadyUsers, (std::uint64_t, std::uint64_t), (override));
};

} // namespace tests
