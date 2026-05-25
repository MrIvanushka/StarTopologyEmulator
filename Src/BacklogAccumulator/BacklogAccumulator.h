#pragma once

#include <cstdint>
#include <map>

#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class BacklogAccumulator : public IBacklogAccumulator
{
public:
	BacklogAccumulator(std::uint64_t bitsPerSlot, MetricScope scope = {});

	void handleReport(std::shared_ptr<BacklogReportMessage>) override;

	std::uint64_t backlogFor(StationID) const override;

	const std::map<StationID, std::uint64_t>& backlogs() const override;

	std::shared_ptr<OperationPlanMessage> generateOperationPlan(
		std::uint64_t frameNumber,
		std::uint64_t totalDedicatedSlots) const override;

private:
	std::uint64_t _bitsPerSlot;
	std::map<StationID, std::uint64_t> _backlogs;
	MetricScope _scope;
	MetricHandle _hTotalBacklog = kInvalidMetricHandle;
	MetricHandle _hDaOccupiedBits = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
