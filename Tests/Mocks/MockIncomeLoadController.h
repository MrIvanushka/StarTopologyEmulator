#pragma once

#include <cstdint>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace tests
{

class MockIncomeLoadController : public starTopologyEmulator::IIncomeLoadController
{
public:
	MOCK_METHOD(starTopologyEmulator::StarHubPlanMessage::BackoffConfig, generate,
		(std::uint64_t, std::uint64_t, std::uint64_t), (override));
};

} // namespace tests
