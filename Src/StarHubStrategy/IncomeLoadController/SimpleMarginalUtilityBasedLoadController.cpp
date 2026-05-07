#include "SimpleMarginalUtilityBasedLoadController.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

SimpleMarginalUtilityBasedLoadController::SimpleMarginalUtilityBasedLoadController(
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	SimpleMarginalUtilityBasedLoadControllerConfig&& config,
	MetricScope scope)
	: _config(std::move(config))
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _readyUsersPredictor(readyUsersPredictor)
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hPTx = _scope.registerMetric("Целевая вероятность вещания");
		_hBackoff = _scope.registerMetric("Целевое окно backoff");
	}
}

StarHubPlanMessage::BackoffConfig SimpleMarginalUtilityBasedLoadController::generate(
	std::uint64_t /*plannedRaSlots*/,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const auto& currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);

	const double currentP = currentPlan->backoff().pTx;
	const std::uint32_t currentBackoff = currentPlan->backoff().baseWindow;

	const double estimatedStations = std::max(
		1.0,
		_readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame));

	const double gradient = utilityGradient(currentP, estimatedStations);

	const double rawDelta = _config.gradientStep * gradient;
	const double limitedDelta = std::max(
		std::min(rawDelta, _config.maxProbabilityStep),
		-_config.maxProbabilityStep);

	StarHubPlanMessage::BackoffConfig result;
	result.pTx = clampProbability(currentP + limitedDelta);
	result.baseWindow = currentBackoff;

	_scope.emit(_hPTx, targetFrame, result.pTx);
	_scope.emit(_hBackoff, targetFrame, static_cast<double>(result.baseWindow));

	return result;
}

double SimpleMarginalUtilityBasedLoadController::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

double SimpleMarginalUtilityBasedLoadController::utilityGradient(
	double txProbability,
	double estimatedStations) const
{
	const double n = std::max(1.0, estimatedStations);
	const double p = clampProbability(txProbability);
	const double q = std::max(_config.epsilon, 1.0 - p);

	const double qPow = std::pow(q, n - 2.0);
	const double bracket =
		_config.weightThroughput * (1.0 - n * p)
		- _config.weightCollision * (n - 1.0) * p;

	return n * qPow * bracket;
}

} // namespace starTopologyEmulator
