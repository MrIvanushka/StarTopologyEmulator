#include "HysteresisLoadController.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

HysteresisLoadController::HysteresisLoadController(
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	HysteresisLoadControllerConfig&& config,
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
		_hState = _scope.registerMetric("Состояние гистерезиса");
	}
}

StarHubPlanMessage::BackoffConfig HysteresisLoadController::generate(
	std::uint64_t plannedRaSlots,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const auto& currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);

	const double predictedReadyUsers = _readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame);

	const double holdLoad = predictLoadUnderCommand(
		predictedReadyUsers,
		plannedRaSlots,
		currentPlan->backoff().pTx,
		currentPlan->backoff().baseWindow);

	_state = nextStateByHysteresis(_state, holdLoad);

	const auto [targetP, targetBackoff] = targetCommandForState(_state);

	const double nextP = smoothProbability(currentPlan->backoff().pTx, targetP);
	const std::uint32_t nextBackoff = smoothBackoff(
		currentPlan->backoff().baseWindow,
		targetBackoff);

	StarHubPlanMessage::BackoffConfig result;
	result.pTx = nextP;
	result.baseWindow = nextBackoff;

	_scope.emit(_hPTx, targetFrame, result.pTx);
	_scope.emit(_hBackoff, targetFrame, static_cast<double>(result.baseWindow));
	_scope.emit(_hState, targetFrame, static_cast<double>(static_cast<int>(_state)));

	return result;
}

double HysteresisLoadController::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

std::uint32_t HysteresisLoadController::clampBackoff(std::uint32_t value) const
{
	return std::max(
		_config.minBackoffWindowFrames,
		std::min(value, _config.maxBackoffWindowFrames));
}

double HysteresisLoadController::backoffEligibilityFactor(std::uint32_t windowFrames) const
{
	const std::uint32_t w = std::max<std::uint32_t>(1U, windowFrames);
	return 2.0 / (static_cast<double>(w) + 1.0);
}

double HysteresisLoadController::effectiveAggressiveness(
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double p = clampProbability(txProbability);
	const double eligibility = backoffEligibilityFactor(backoffWindowFrames);
	return std::max(_config.epsilon, p * eligibility);
}

double HysteresisLoadController::predictLoadUnderCommand(
	double predictedReadyUsers,
	std::uint64_t raSlots,
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double aggressiveness = effectiveAggressiveness(
		txProbability,
		backoffWindowFrames);

	const double r = static_cast<double>(std::max<std::uint64_t>(1ULL, raSlots));
	return std::max(0.0, predictedReadyUsers * aggressiveness / r);
}

HysteresisLoadController::State HysteresisLoadController::nextStateByHysteresis(
	State current,
	double predictedHoldLoad) const
{
	switch (current)
	{
	case State::Normal:
		if (predictedHoldLoad >= _config.gHigh)
			return State::Critical;
		return State::Normal;

	case State::Critical:
		if (predictedHoldLoad <= _config.gLow)
			return State::Normal;
		return State::Critical;
	}

	return current;
}

std::pair<double, std::uint32_t> HysteresisLoadController::targetCommandForState(
	State state) const
{
	switch (state)
	{
	case State::Normal:
		return {
			clampProbability(_config.pNormal),
			clampBackoff(_config.backoffNormalFrames)
		};

	case State::Critical:
		return {
			clampProbability(_config.pCritical),
			clampBackoff(_config.backoffCriticalFrames)
		};
	}

	return {
		clampProbability(_config.pNormal),
		clampBackoff(_config.backoffNormalFrames)
	};
}

double HysteresisLoadController::smoothProbability(double current, double target) const
{
	const double alpha = std::max(std::min(_config.alphaProbability, 1.0), 0.0);
	const double blended = current + alpha * (target - current);

	const double delta = blended - current;
	const double limitedDelta = std::max(
		std::min(delta, _config.maxProbabilityStep),
		-_config.maxProbabilityStep);

	return clampProbability(current + limitedDelta);
}

std::uint32_t HysteresisLoadController::smoothBackoff(
	std::uint32_t current,
	std::uint32_t target) const
{
	const double alpha = std::max(std::min(_config.alphaBackoff, 1.0), 0.0);
	const double currentD = static_cast<double>(current);
	const double targetD = static_cast<double>(target);

	const double blended = currentD + alpha * (targetD - currentD);
	const double delta = blended - currentD;

	const double limitedDelta = std::max(
		std::min(delta, static_cast<double>(_config.maxBackoffStepFrames)),
		-static_cast<double>(_config.maxBackoffStepFrames));

	const double nextValue = currentD + limitedDelta;
	const auto rounded = static_cast<std::uint32_t>(std::llround(nextValue));

	return clampBackoff(rounded);
}

} // namespace starTopologyEmulator
