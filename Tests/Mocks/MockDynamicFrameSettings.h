#pragma once

#include <cstdint>
#include <memory>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/Messages/OperationPlanMessage.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"

namespace tests
{

class MockDynamicFrameSettings : public starTopologyEmulator::IDynamicFrameSettings
{
public:
	MOCK_METHOD(void, handlePlan,
		(std::shared_ptr<starTopologyEmulator::StarHubPlanMessage>), (override));
	MOCK_METHOD(void, handleOperationPlan,
		(std::shared_ptr<starTopologyEmulator::OperationPlanMessage>), (override));
	MOCK_METHOD(void, clearOutdated, (std::uint64_t), (override));
	MOCK_METHOD(std::shared_ptr<starTopologyEmulator::StarHubPlanMessage>, currentPlan,
		(std::uint64_t), (const, override));
	MOCK_METHOD(std::shared_ptr<starTopologyEmulator::OperationPlanMessage>, currentOperationPlan,
		(std::uint64_t), (const, override));
	MOCK_METHOD(std::uint64_t, earliestPlanNumber, (), (const, override));
};

} // namespace tests
