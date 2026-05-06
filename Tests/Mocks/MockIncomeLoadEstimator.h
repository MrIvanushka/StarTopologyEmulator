#pragma once

#include <gmock/gmock.h>

#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"

namespace tests
{

class MockIncomeLoadEstimator : public starTopologyEmulator::IIncomeLoadEstimator
{
public:
	MOCK_METHOD(void, update, (const starTopologyEmulator::RandomAccessFrameResult&), (override));
	MOCK_METHOD(double, incomeLoad, (), (const, override));
	MOCK_METHOD(double, plr, (), (const, override));
	MOCK_METHOD(void, reset, (), (override));
};

} // namespace tests
