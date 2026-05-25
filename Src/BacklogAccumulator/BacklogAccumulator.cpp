#include "BacklogAccumulator.h"

#include <algorithm>
#include <vector>

namespace starTopologyEmulator
{

BacklogAccumulator::BacklogAccumulator(std::uint64_t bitsPerSlot, MetricScope scope)
	: _bitsPerSlot(bitsPerSlot)
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hTotalBacklog = _scope.registerMetric("Суммарный бэклог абонентских станций");
		_hDaOccupiedBits = _scope.registerMetric("Объём трафика в DA-сегменте");
	}
}

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

	_scope.emit(_hTotalBacklog, frameNumber, static_cast<double>(totalBacklog));
	_scope.emit(_hDaOccupiedBits, frameNumber, static_cast<double>(std::min(totalBacklog, totalBits)));

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
