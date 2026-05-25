#include "PiLoadController.h"

#include <algorithm>

#include "AntiWindup/AntiWindupFactory.h"

namespace starTopologyEmulator
{

PiLoadController::PiLoadController(
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	PiLoadControllerConfig&& config,
	MetricScope scope)
	: _config(std::move(config))
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _readyUsersPredictor(readyUsersPredictor)
	, _antiWindup(AntiWindupFactory::make(_config.antiWindup))
	, _leakFactor(_config.integralWindowFrames > 0
		? 1.0 / static_cast<double>(_config.integralWindowFrames)
		: 0.0)
	, _lastOutput(_config.backoffTemplate)
	, _scope(std::move(scope))
{
	_lastOutput.pTx = clampProbability(_lastOutput.pTx);

	if (_scope.active())
	{
		_hPTx = _scope.registerMetric("Вероятность вещания");
		_hError = _scope.registerMetric("Ошибка нагрузки");
		_hLoad = _scope.registerMetric("Оценка входной нагрузки");
	}
}

StarHubPlanMessage::BackoffConfig PiLoadController::generate(
	std::uint64_t plannedRaSlots,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const auto currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);

	const double currentPTx = currentPlan
		? currentPlan->backoff().pTx
		: _lastOutput.pTx;
	const std::uint32_t currentBaseWindow = currentPlan
		? currentPlan->backoff().baseWindow
		: _lastOutput.baseWindow;

	const double readyUsers = _readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame);

	const double estimatedLoad = estimateLoad(
		readyUsers,
		currentPTx,
		currentBaseWindow,
		plannedRaSlots);

	const double error = _config.gTarget - estimatedLoad;

	const double u = _config.kP * error + _integral;
	const double delta = clampStep(_config.alpha * u);

	const double pUnclamped = (_config.allowFeedForward ? plannedRaSlots / readyUsers : currentPTx) + delta;
	const double pClamped = clampProbability(pUnclamped);

	const IAntiWindup::Step antiWindupStep{
		_integral,
		pUnclamped,
		pClamped,
		error,
		_config.kP,
		_config.kI,
	};
	const double correction = _antiWindup->correction(antiWindupStep);

	_integral = (1.0 - _leakFactor) * _integral + _config.kI * error - correction;

	StarHubPlanMessage::BackoffConfig result = _config.backoffTemplate;
	result.pTx = pClamped;

	_lastOutput = result;

	_scope.emit(_hPTx, targetFrame, result.pTx);
	_scope.emit(_hError, targetFrame, error);
	_scope.emit(_hLoad, targetFrame, estimatedLoad);

	return result;
}

double PiLoadController::estimateLoad(
	double readyUsers,
	double currentPTx,
	std::uint32_t currentBaseWindow,
	std::uint64_t plannedRaSlots) const
{
	const double w = static_cast<double>(std::max<std::uint32_t>(1U, currentBaseWindow));
	const double aggressiveness = std::max(_config.epsilon, 2.0 * currentPTx / (w + 1.0));
	const double r = static_cast<double>(std::max<std::uint64_t>(1ULL, plannedRaSlots));
	return std::max(0.0, readyUsers * aggressiveness / r);
}

double PiLoadController::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

double PiLoadController::clampStep(double delta) const
{
	return std::max(-_config.maxProbabilityStep, std::min(delta, _config.maxProbabilityStep));
}

} // namespace starTopologyEmulator
