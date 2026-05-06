#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IMessage.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT OperationPlanMessage : public IMessage
{
public:
	struct StationAllocation
	{
		StationID stationID;
		std::uint64_t allocatedBits;
	};

	OperationPlanMessage(std::uint64_t frame, std::vector<StationAllocation> allocations)
		: _frame(frame), _allocations(std::move(allocations))
	{}

	MessageType type() const override { return MessageType::OperationPlan; }

	std::uint64_t frame() const { return _frame; }
	const std::vector<StationAllocation>& allocations() const { return _allocations; }

	std::uint64_t allocationFor(StationID id) const
	{
		for (const auto& a : _allocations)
			if (a.stationID == id)
				return a.allocatedBits;
		return 0;
	}

private:
	std::uint64_t _frame;
	std::vector<StationAllocation> _allocations;
};

} // namespace starTopologyEmulator
