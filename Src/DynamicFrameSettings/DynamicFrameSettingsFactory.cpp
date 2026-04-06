#pragma once

#include <deque>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"

namespace starTopologyEmulator
{

class DynamicFrameSettings : public IDynamicFrameSettings
{
public:
	DynamicFrameSettings(int maxPlansStored = 1'000);
	
	void handlePlan(std::shared_ptr<StarHubPlanMessage>) override;

	void clearOutdated(std::uint64_t frame) override;

	std::shared_ptr<StarHubPlanMessage> currentPlan(std::uint64_t frame) const override;
private:
	const int _maxPlansStored;

	std::deque<std::shared_ptr<StarHubPlanMessage>> _plans;
};

} // namespace starTopologyEmulator
