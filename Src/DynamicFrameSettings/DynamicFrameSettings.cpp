#include "DynamicFrameSettings/DynamicFrameSettings.h"

namespace starTopologyEmulator
{

DynamicFrameSettings::DynamicFrameSettings(int maxPlansStored)
	: _maxPlansStored(maxPlansStored)
{}

void DynamicFrameSettings::handlePlan(std::shared_ptr<StarHubPlanMessage> message)
{
	_plans.push_back(message);
	while (_plans.size() > _maxPlansStored)
		_plans.pop_front();
}

void DynamicFrameSettings::clearOutdated(std::uint64_t frame)
{
	while (!_plans.empty())
	{
		if (_plans.front()->frame() < frame)
			_plans.pop_front();
		else
			break;
	}
}

std::uint64_t DynamicFrameSettings::earliestPlanNumber() const
{
	return _plans.front()->frame();
}

} // namespace starTopologyEmulator
