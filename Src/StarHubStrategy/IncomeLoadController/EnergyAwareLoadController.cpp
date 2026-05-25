#include "EnergyAwareLoadController.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

EnergyAwareLoadController::EnergyAwareLoadController(
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	EnergyAwareLoadControllerConfig&& config,
	MetricScope scope)
	: _config(std::move(config))
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _readyUsersPredictor(readyUsersPredictor)
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hPTx = _scope.registerMetric("Вероятность вещания");
		_hBackoff = _scope.registerMetric("Окно backoff");
	}
}

StarHubPlanMessage::BackoffConfig EnergyAwareLoadController::generate(
	std::uint64_t /*plannedRaSlots*/,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const auto& currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);

	const double currentP = currentPlan->backoff().pTx;
	const std::uint8_t currentBackoff = currentPlan->backoff().baseWindow;

	const double n = std::max(1.0, _readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame));

	const double gradient = utilityGradient(currentP, n);
	const double rawDelta = _config.gradientStep * gradient;
	const double delta = clampStep(rawDelta);

	StarHubPlanMessage::BackoffConfig result = _config.backoffTemplate;
	result.pTx = clampProbability(currentP + delta);
	result.baseWindow = currentBackoff;

	_scope.emit(_hPTx, targetFrame, result.pTx);
	_scope.emit(_hBackoff, targetFrame, static_cast<double>(result.baseWindow));

	return result;
}

double EnergyAwareLoadController::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

double EnergyAwareLoadController::clampStep(double delta) const
{
	return std::max(-_config.maxProbabilityStep, std::min(delta, _config.maxProbabilityStep));
}

double EnergyAwareLoadController::utilityGradient(double pTx, double n) const
{
	const double p = clampProbability(pTx);
	const double q = std::max(_config.epsilon, 1.0 - p);

	const double dS = n * std::pow(q, n - 2.0) * (1.0 - n * p);

	return _config.weightThroughput * dS - _config.weightEnergy * n;
}

} // namespace starTopologyEmulator
