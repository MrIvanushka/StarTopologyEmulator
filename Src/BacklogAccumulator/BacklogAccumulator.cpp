#include "BacklogAccumulator.h"

#include <vector>

namespace starTopologyEmulator
{

BacklogAccumulator::BacklogAccumulator(std::uint64_t bitsPerSlot)
	: _bitsPerSlot(bitsPerSlot)
{}

void BacklogAccumulator::handleReport(std::shared_ptr<BacklogReportMessage> report)
{
	if (!report) return;
	_backlogs[report->stationID()] = report->backlogBits();
}

std::uint64_t BacklogAccumulator::backlogFor(StationID id) const
{
	auto it = _backlogs.find(id);
	return it != _backlogs.end() ? it->second : 0;
}

const std::map<StationID, std::uint64_t>& BacklogAccumulator::backlogs() const
{
	return _backlogs;
}

std::shared_ptr<OperationPlanMessage> BacklogAccumulator::generateOperationPlan(
	std::uint64_t frameNumber,
	std::uint64_t totalDedicatedSlots) const
{
	std::vector<OperationPlanMessage::StationAllocation> allocations;
	allocations.reserve(_backlogs.size());

	const std::uint64_t totalBits = totalDedicatedSlots * _bitsPerSlot;

	std::uint64_t totalBacklog = 0;
	for (const auto& [_, bits] : _backlogs)
		totalBacklog += bits;

	if (totalBits == 0 || totalBacklog == 0)
	{
		for (const auto& [id, _] : _backlogs)
			allocations.push_back({ id, 0 });
	}
	else if (totalBacklog <= totalBits)
	{
		// Demand fits the available throughput: every station gets its full backlog.
		for (const auto& [id, bits] : _backlogs)
			allocations.push_back({ id, bits });
	}
	else
	{
		// Proportional split. Use double to avoid 64-bit overflow on the
		// totalBits * bits intermediate; backlog cap is implicit since
		// totalBacklog > totalBits.
		for (const auto& [id, bits] : _backlogs)
		{
			const auto alloc = static_cast<std::uint64_t>(
				static_cast<double>(totalBits) *
				static_cast<double>(bits) /
				static_cast<double>(totalBacklog));
			allocations.push_back({ id, alloc });
		}
	}

	return std::make_shared<OperationPlanMessage>(frameNumber, std::move(allocations));
}

} // namespace starTopologyEmulator
