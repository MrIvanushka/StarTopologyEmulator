#include "RiskSensitiveLoadController.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

RiskSensitiveLoadController::RiskSensitiveLoadController(
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	RiskSensitiveLoadControllerConfig&& config,
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

StarHubPlanMessage::BackoffConfig RiskSensitiveLoadController::generate(
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

	StarHubPlanMessage::BackoffConfig result;
	result.pTx = clampProbability(currentP + delta);
	result.baseWindow = currentBackoff;

	_scope.emit(_hPTx, targetFrame, result.pTx);
	_scope.emit(_hBackoff, targetFrame, static_cast<double>(result.baseWindow));

	return result;
}

double RiskSensitiveLoadController::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

double RiskSensitiveLoadController::clampStep(double delta) const
{
	return std::max(-_config.maxProbabilityStep, std::min(delta, _config.maxProbabilityStep));
}

double RiskSensitiveLoadController::utilityGradient(double pTx, double n) const
{
	const double p = clampProbability(pTx);
	const double q = std::max(_config.epsilon, 1.0 - p);
	const double beta = std::max(_config.epsilon, _config.beta);

	const double qPowN = std::pow(q, n);
	const double qPowN1 = std::pow(q, n - 1.0);
	const double qPowN2 = std::pow(q, n - 2.0);

	const double S = n * p * qPowN1;
	const double pColl = std::max(0.0, 1.0 - qPowN - S);
	const double dS = n * qPowN2 * (1.0 - n * p);
	const double dPColl = n * (n - 1.0) * p * qPowN2;

	const double expSucc = std::exp(-beta);
	const double expColl = std::exp(beta * _config.collisionPenalty);

	const double M = std::max(_config.epsilon, S * expSucc + qPowN + pColl * expColl);
	const double dM = expSucc * dS - n * qPowN1 + expColl * dPColl;

	return -(1.0 / beta) * dM / M;
}

} // namespace starTopologyEmulator
