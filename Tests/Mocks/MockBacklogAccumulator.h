#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include <gmock/gmock.h>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/Messages/BacklogReportMessage.h"
#include "StarTopologyEmulator/Messages/OperationPlanMessage.h"

namespace tests
{

class MockBacklogAccumulator : public starTopologyEmulator::IBacklogAccumulator
{
public:
	MOCK_METHOD(void, handleReport,
		(std::shared_ptr<starTopologyEmulator::BacklogReportMessage>), (override));
	MOCK_METHOD(std::uint64_t, backlogFor, (starTopologyEmulator::StationID), (const, override));
	MOCK_METHOD((const std::map<starTopologyEmulator::StationID, std::uint64_t>&), backlogs,
		(), (const, override));
	MOCK_METHOD(std::shared_ptr<starTopologyEmulator::OperationPlanMessage>, generateOperationPlan,
		(std::uint64_t, std::uint64_t), (const, override));
};

} // namespace tests
