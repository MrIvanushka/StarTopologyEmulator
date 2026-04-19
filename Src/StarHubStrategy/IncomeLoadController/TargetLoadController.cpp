#include "TargetLoadController.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

TargetLoadController::TargetLoadController(
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	TargetLoadControllerConfig&& config)
	: _config(std::move(config))
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _readyUsersPredictor(readyUsersPredictor)
{
	REGISTER_METRIC_SUBFOLDER(_readyUsersPredictor.get());
	REGISTER_METRIC(_currentPlan ? _currentPlan->backoff().pTx : 0, "“екуща€ веро€тность вещани€");
	REGISTER_METRIC(_currentPlan ? _currentPlan->backoff().baseWindow : 0, "“екуща€ ширина окна backoff");
}

StarHubPlanMessage::BackoffConfig TargetLoadController::generate(
	std::uint64_t plannedRaSlots,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const auto& currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);
	_currentPlan = currentPlan;
	if (!currentPlan)
		return {};

	const double predictedReadyUsers =
		_readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame);

	const double holdLoad = predictLoadUnderCommand(
		predictedReadyUsers,
		plannedRaSlots,
		currentPlan->backoff().pTx,
		currentPlan->backoff().baseWindow);

	double targetP = currentPlan->backoff().pTx;
	std::uint32_t targetBackoff = currentPlan->backoff().baseWindow;

	const double low = std::max(0.0, _config.gTarget - _config.gDeadBand);
	const double high = _config.gTarget + _config.gDeadBand;

	if (holdLoad < low || holdLoad > high)
	{
		const double targetAggressiveness = computeTargetAggressiveness(
			predictedReadyUsers,
			plannedRaSlots);

		const auto [p, backoff] = commandForAggressiveness(targetAggressiveness);
		targetP = p;
		targetBackoff = backoff;
	}

	const double nextP = smoothProbability(currentPlan->backoff().pTx, targetP);
	const std::uint32_t nextBackoff = smoothBackoff(
		currentPlan->backoff().baseWindow,
		targetBackoff);

	StarHubPlanMessage::BackoffConfig result;
	result.pTx = nextP;
	result.baseWindow = nextBackoff;

	return result;
}

double TargetLoadController::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

std::uint32_t TargetLoadController::clampBackoff(std::uint32_t value) const
{
	return std::max(
		_config.minBackoffWindowFrames,
		std::min(value, _config.maxBackoffWindowFrames));
}

double TargetLoadController::backoffEligibilityFactor(std::uint32_t windowFrames) const
{
	const std::uint32_t w = std::max<std::uint32_t>(1U, windowFrames);
	return 2.0 / (static_cast<double>(w) + 1.0);
}

double TargetLoadController::effectiveAggressiveness(
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double p = clampProbability(txProbability);
	const double eligibility = backoffEligibilityFactor(backoffWindowFrames);
	return std::max(_config.epsilon, p * eligibility);
}

double TargetLoadController::minAggressiveness() const
{
	return effectiveAggressiveness(
		_config.minProbability,
		_config.maxBackoffWindowFrames);
}

double TargetLoadController::maxAggressiveness() const
{
	return effectiveAggressiveness(
		_config.maxProbability,
		_config.minBackoffWindowFrames);
}

double TargetLoadController::predictLoadUnderCommand(
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

double TargetLoadController::computeTargetAggressiveness(
	double predictedReadyUsers,
	std::uint64_t plannedRaSlots) const
{
	if (predictedReadyUsers <= _config.epsilon)
		return maxAggressiveness();

	const double r = static_cast<double>(std::max<std::uint64_t>(1ULL, plannedRaSlots));
	const double rawAggressiveness = _config.gTarget * r / predictedReadyUsers;

	return std::max(
		minAggressiveness(),
		std::min(rawAggressiveness, maxAggressiveness()));
}

std::pair<double, std::uint32_t> TargetLoadController::commandForAggressiveness(
	double targetAggressiveness) const
{
	const double aggressiveness = std::max(
		minAggressiveness(),
		std::min(targetAggressiveness, maxAggressiveness()));

	const std::uint32_t minBackoff = clampBackoff(_config.minBackoffWindowFrames);
	const std::uint32_t maxBackoff = clampBackoff(_config.maxBackoffWindowFrames);

	const double minWindowEligibility = backoffEligibilityFactor(minBackoff);
	const double pAtMinWindow = aggressiveness / minWindowEligibility;

	// —начала пытаемс€ удерживать цель только веро€тностью вещани€
	// при минимальном окне backoff.
	if (pAtMinWindow >= _config.minProbability)
	{
		return {
			clampProbability(pAtMinWindow),
			minBackoff
		};
	}

	// ≈сли этого уже недостаточно, фиксируем веро€тность на минимуме
	// и расшир€ем окно backoff.
	const double fixedProbability = clampProbability(_config.minProbability);

	const double desiredEligibility = std::max(
		backoffEligibilityFactor(maxBackoff),
		std::min(
			aggressiveness / fixedProbability,
			backoffEligibilityFactor(minBackoff)));

	// phi(W) = 2 / (W + 1)
	// => W = 2 / phi - 1
	const double rawWindow = (2.0 / desiredEligibility) - 1.0;
	const auto backoff = clampBackoff(
		static_cast<std::uint32_t>(std::llround(rawWindow)));

	return {
		fixedProbability,
		backoff
	};
}

double TargetLoadController::smoothProbability(double current, double target) const
{
	const double alpha = std::max(std::min(_config.alphaProbability, 1.0), 0.0);
	const double blended = current + alpha * (target - current);

	const double delta = blended - current;
	const double limitedDelta = std::max(
		std::min(delta, _config.maxProbabilityStep),
		-_config.maxProbabilityStep);

	return clampProbability(current + limitedDelta);
}

std::uint32_t TargetLoadController::smoothBackoff(
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