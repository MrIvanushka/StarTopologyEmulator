#pragma once

#include <cstdint>
#include <map>
#include <memory>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/Messages/BacklogReportMessage.h"
#include "StarTopologyEmulator/Messages/OperationPlanMessage.h"

namespace starTopologyEmulator
{

class IBacklogAccumulator
{
public:
	virtual ~IBacklogAccumulator() = default;

	virtual void handleReport(std::shared_ptr<BacklogReportMessage>) = 0;

	virtual std::uint64_t backlogFor(StationID) const = 0;

	virtual const std::map<StationID, std::uint64_t>& backlogs() const = 0;

	virtual std::shared_ptr<OperationPlanMessage> generateOperationPlan(
		std::uint64_t frameNumber,
		std::uint64_t totalDedicatedSlots) const = 0;
};

} // namespace starTopologyEmulator
