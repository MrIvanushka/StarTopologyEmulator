#pragma once

#include <cstdint>
#include <memory>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace tests
{

class MockStarHubStrategy : public starTopologyEmulator::IStarHubStrategy
{
public:
	MOCK_METHOD(std::shared_ptr<starTopologyEmulator::StarHubPlanMessage>, generate,
		(std::uint64_t, std::uint64_t), (override));
};

} // namespace tests
