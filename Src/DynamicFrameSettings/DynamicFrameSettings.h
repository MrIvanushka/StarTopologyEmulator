#pragma once

#include <deque>

#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"

namespace starTopologyEmulator
{

class DynamicFrameSettings : public IDynamicFrameSettings
{
public:
	DynamicFrameSettings(int maxPlansStored = 10'000);

	void handlePlan(std::shared_ptr<StarHubPlanMessage>) override;
	void handleOperationPlan(std::shared_ptr<OperationPlanMessage>) override;

	void clearOutdated(std::uint64_t frame) override;

	std::shared_ptr<StarHubPlanMessage> currentPlan(std::uint64_t frame) const override;
	std::shared_ptr<OperationPlanMessage> currentOperationPlan(std::uint64_t frame) const override;

	std::uint64_t earliestPlanNumber() const override;
private:
	const int _maxPlansStored;

	std::deque<std::shared_ptr<StarHubPlanMessage>> _plans;
	std::deque<std::shared_ptr<OperationPlanMessage>> _operationPlans;
};

} // namespace starTopologyEmulator
