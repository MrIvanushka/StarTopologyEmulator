#include "StaticIncomeStationsPredictor.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

StaticIncomeStationsPredictor::StaticIncomeStationsPredictor(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IFrameCalculator> frameCalculator,
	StaticIncomeStationsPredictorConfig&& config,
	MetricScope scope)
	: _config(std::move(config))
	, _incomeLoadEstimator(incomeLoadEstimator)
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _frameCalculator(frameCalculator)
	, _scope(std::move(scope))
{
	if (_scope.active())
		_hReadyUsers = _scope.registerMetric("Оценка количества станций в RA");
}

double StaticIncomeStationsPredictor::estimateReadyUsers(
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const double currentIncomeLoad = std::max(0.0, _incomeLoadEstimator->incomeLoad());
	_incomeLoadHistory[currentFrame] = currentIncomeLoad;

	const auto& currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);
	if (!currentPlan)
	{
		_scope.emit(_hReadyUsers, targetFrame, 0.0);
		return 0.0;
	}

	const auto readyUsers = historyReadyUsers(currentPlan);
	const double result = readyUsers.value_or(0.0);

	_scope.emit(_hReadyUsers, targetFrame, result);
	return result;
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
