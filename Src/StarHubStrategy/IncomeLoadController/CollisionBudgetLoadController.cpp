#include "CollisionBudgetLoadController.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

CollisionBudgetLoadController::CollisionBudgetLoadController(
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	CollisionBudgetLoadControllerConfig&& config,
	MetricScope scope)
	: _config(std::move(config))
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _readyUsersPredictor(readyUsersPredictor)
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hPTx = _scope.registerMetric("Вероятность вещания");
		_hMu = _scope.registerMetric("Лагранжев множитель");
		_hPColl = _scope.registerMetric("Доля коллизий");
	}
}

StarHubPlanMessage::BackoffConfig CollisionBudgetLoadController::generate(
	std::uint64_t /*plannedRaSlots*/,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const auto currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);

	const double currentPTx = currentPlan
		? currentPlan->backoff().pTx
		: _config.backoffTemplate.pTx;

	const double n = std::max(1.0, _readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame));

	const double pColl = collisionRate(currentPTx, n);
	const double constraintGap = pColl - _config.collisionBudget;

	const double dS = throughputDerivative(currentPTx, n);
	const double dPColl = collisionRateDerivative(currentPTx, n);

	const double gradient = (constraintGap > 0.0)
		? (dS - _lagrangeMultiplier * dPColl)
		: dS;

	const double rawDelta = _config.gradientStep * gradient;
	const double delta = clampStep(rawDelta);

	StarHubPlanMessage::BackoffConfig result = _config.backoffTemplate;
	result.pTx = clampProbability(currentPTx + delta);

	_lagrangeMultiplier = std::max(
		0.0,
		_lagrangeMultiplier + _config.lagrangianStep * constraintGap);

	_scope.emit(_hPTx, targetFrame, result.pTx);
	_scope.emit(_hMu, targetFrame, _lagrangeMultiplier);
	_scope.emit(_hPColl, targetFrame, pColl);

	return result;
}

double CollisionBudgetLoadController::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

double CollisionBudgetLoadController::clampStep(double delta) const
{
	return std::max(-_config.maxProbabilityStep, std::min(delta, _config.maxProbabilityStep));
}

double CollisionBudgetLoadController::collisionRate(double p, double n) const
{
	const double q = std::max(_config.epsilon, 1.0 - p);
	const double qPow = std::pow(q, n);
	const double success = n * p * std::pow(q, n - 1.0);
	return std::max(0.0, 1.0 - qPow - success);
}

double CollisionBudgetLoadController::throughputDerivative(double p, double n) const
{
	const double q = std::max(_config.epsilon, 1.0 - p);
	return n * std::pow(q, n - 2.0) * (1.0 - n * p);
}

double CollisionBudgetLoadController::collisionRateDerivative(double p, double n) const
{
	const double q = std::max(_config.epsilon, 1.0 - p);
	return n * (n - 1.0) * p * std::pow(q, n - 2.0);
}

} // namespace starTopologyEmulator
