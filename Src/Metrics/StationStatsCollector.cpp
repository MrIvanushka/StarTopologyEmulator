#include "StarTopologyEmulator/Metrics/StationStatsCollector.h"

namespace starTopologyEmulator
{

StationStatsCollector::StationStatsCollector(MetricScope scope)
	: _scope(std::move(scope))
{
	if (_scope.active())
		_hPtxCheckCount = _scope.registerMetric("Станции, проверившие p_tx");
}

void StationStatsCollector::onPtxCheck(std::uint64_t frame)
{
	rollOverIfNeeded(frame);
	++_ptxCheckCount;
}

void StationStatsCollector::rollOverIfNeeded(std::uint64_t frame)
{
	if (_currentFrame && *_currentFrame != frame)
	{
		for (auto logFrame = *_currentFrame; logFrame < frame; ++logFrame)
		{
			_scope.emit(_hPtxCheckCount, logFrame, static_cast<double>(_ptxCheckCount));
			_ptxCheckCount = 0;
		}
	}
	_currentFrame = frame;
}

} // namespace starTopologyEmulator
