#include "StaticIncomeStationsPredictor.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

StaticIncomeStationsPredictor::StaticIncomeStationsPredictor(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IFrameCalculator> frameCalculator,
	StaticIncomeStationsPredictorConfig&& config)
	: _config(std::move(config))
	, _incomeLoadEstimator(incomeLoadEstimator)
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _frameCalculator(frameCalculator)
{
	REGISTER_METRIC(_currentEstimationResult, "ќценка количества станций в RA");
}

double StaticIncomeStationsPredictor::estimateReadyUsers(
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	(void)targetFrame;

	const double currentIncomeLoad = std::max(0.0, _incomeLoadEstimator->incomeLoad());
	_incomeLoadHistory[currentFrame] = currentIncomeLoad;

	const auto& currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);
	if (!currentPlan)
	{
		_currentEstimationResult = 0.0;
		return 0.0;
	}

	const auto readyUsers = historyReadyUsers(currentPlan);
	_currentEstimationResult = readyUsers.value_or(0.0);

	return _currentEstimationResult;
}

double StaticIncomeStationsPredictor::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

double StaticIncomeStationsPredictor::backoffEligibilityFactor(std::uint32_t windowFrames) const
{
	const std::uint32_t w = std::max<std::uint32_t>(1U, windowFrames);
	return 2.0 / (static_cast<double>(w) + 1.0);
}

double StaticIncomeStationsPredictor::effectiveAggressiveness(
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double p = clampProbability(txProbability);
	const double eligibility = backoffEligibilityFactor(backoffWindowFrames);
	return std::max(_config.epsilon, p * eligibility);
}

std::optional<double> StaticIncomeStationsPredictor::historyReadyUsers(
	const std::shared_ptr<StarHubPlanMessage>& plan) const
{
	if (!plan)
		return std::nullopt;

	const auto historyIt = _incomeLoadHistory.find(plan->frame());
	if (historyIt == _incomeLoadHistory.end())
		return std::nullopt;

	const std::uint32_t raSlots = std::max<std::uint32_t>(
		1U,
		static_cast<std::uint32_t>(plan->randomAccessSlotsCountInFrame()));

	const double aggressiveness = effectiveAggressiveness(
		plan->backoff().pTx,
		plan->backoff().baseWindow);

	const double g = std::max(0.0, historyIt->second);

	return g * static_cast<double>(raSlots) / aggressiveness;
}

} // namespace starTopologyEmulator