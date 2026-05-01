#include "CogorthyIncomeStationsPredictor.h"

namespace starTopologyEmulator
{

CogorthyIncomeStationsPredictor::CogorthyIncomeStationsPredictor(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings)
	: _incomeLoadEstimator(incomeLoadEstimator)
	, _dynamicFrameSettings(dynamicFrameSettings)
{}

double CogorthyIncomeStationsPredictor::estimateReadyUsers(
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	_incomeLoadHistory[currentFrame] = _incomeLoadEstimator->incomeLoad();
	const std::uint64_t earliestFrame = _dynamicFrameSettings->earliestPlanNumber();
	if (currentFrame < earliestFrame)
		return 0.0;

	auto res = 0;

	for (auto i = currentFrame; i > 0; --i)
	{
		auto frameImpact = calculateImpact(i, targetFrame, targetFrame - currentFrame);
		res += frameImpact;

		if (frameImpact < 0.01)
			return res;
	}
}

double CogorthyIncomeStationsPredictor::calculateImpact(
	std::uint64_t impactFrame,
	std::uint64_t targetFrame,
	std::uint64_t commandApplyDelay)
{
	auto impactFramePlan = _dynamicFrameSettings->currentPlan(impactFrame);
	if (!impactFramePlan)
		return 0;

	auto res = _incomeLoadHistory[impactFrame] * impactFramePlan->randomAccessSlotsCountInFrame();

	for (auto i = impactFrame + commandApplyDelay; i < targetFrame - 1; ++i)
	{
		auto probability = _dynamicFrameSettings->currentPlan(impactFrame)->backoff().pTx;
		res *= (1 - probability);
	}
	return res;
}

} // namespace starTopologyEmulator
