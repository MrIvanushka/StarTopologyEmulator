#include "SimpleMarginalUtilityBasedLoadController.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace starTopologyEmulator
{

SimpleMarginalUtilityBasedLoadController::SimpleMarginalUtilityBasedLoadController(
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	SimpleMarginalUtilityBasedLoadControllerConfig&& config)
	: _config(std::move(config))
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _readyUsersPredictor(readyUsersPredictor)
{}

StarHubPlanMessage::BackoffConfig SimpleMarginalUtilityBasedLoadController::generate(
	std::uint64_t plannedRaSlots,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const auto& currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);

	const double currentP = currentPlan->backoff().pTx;
	const std::uint32_t currentBackoff = currentPlan->backoff().baseWindow;

	const double predictedReadyUsers =
		_readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame);

	double bestP = currentP;
	std::uint32_t bestBackoff = currentBackoff;
	double bestUtility = utility(
		predictedReadyUsers,
		plannedRaSlots,
		currentP,
		currentBackoff);

	const auto backoffWindows = candidateBackoffWindows(currentBackoff);

	for (const auto backoffWindow : backoffWindows)
	{
		for (double p = _config.minProbability;
			p <= _config.maxProbability + _config.epsilon;
			p += _config.probabilityGridStep)
		{
			const double candidateP = clampProbability(p);
			const double candidateUtility = utility(
				predictedReadyUsers,
				plannedRaSlots,
				candidateP,
				backoffWindow);

			if (candidateUtility > bestUtility + _config.minUtilityGain)
			{
				bestUtility = candidateUtility;
				bestP = candidateP;
				bestBackoff = backoffWindow;
			}
		}
	}

	const double nextP = smoothProbability(currentP, bestP);
	const std::uint32_t nextBackoff = smoothBackoff(currentBackoff, bestBackoff);

	StarHubPlanMessage::BackoffConfig result;
	result.pTx = nextP;
	result.baseWindow = nextBackoff;

	return result;
}

double SimpleMarginalUtilityBasedLoadController::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

std::uint32_t SimpleMarginalUtilityBasedLoadController::clampBackoff(std::uint32_t value) const
{
	return std::max(
		_config.minBackoffWindowFrames,
		std::min(value, _config.maxBackoffWindowFrames));
}

double SimpleMarginalUtilityBasedLoadController::backoffEligibilityFactor(std::uint32_t windowFrames) const
{
	const std::uint32_t w = std::max<std::uint32_t>(1U, windowFrames);
	return 2.0 / (static_cast<double>(w) + 1.0);
}

double SimpleMarginalUtilityBasedLoadController::effectiveAggressiveness(
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double p = clampProbability(txProbability);
	const double eligibility = backoffEligibilityFactor(backoffWindowFrames);
	return std::max(_config.epsilon, p * eligibility);
}

double SimpleMarginalUtilityBasedLoadController::predictLoadUnderCommand(
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

double SimpleMarginalUtilityBasedLoadController::successPerSlotSa(double g) const
{
	const double load = std::max(0.0, g);
	return load * std::exp(-load);
}

double SimpleMarginalUtilityBasedLoadController::collisionProbabilitySa(double g) const
{
	const double load = std::max(0.0, g);
	const double idle = std::exp(-load);
	const double success = load * idle;
	return std::max(0.0, 1.0 - idle - success);
}

double SimpleMarginalUtilityBasedLoadController::predictedDelayFrames(
	double predictedReadyUsers,
	std::uint64_t raSlots,
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double g = predictLoadUnderCommand(
		predictedReadyUsers,
		raSlots,
		txProbability,
		backoffWindowFrames);

	const double successPerSlot = successPerSlotSa(g);
	const double successPerFrame =
		static_cast<double>(std::max<std::uint64_t>(1ULL, raSlots)) * successPerSlot;

	return predictedReadyUsers / std::max(successPerFrame, _config.epsilon);
}

double SimpleMarginalUtilityBasedLoadController::utility(
	double predictedReadyUsers,
	std::uint64_t raSlots,
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double g = predictLoadUnderCommand(
		predictedReadyUsers,
		raSlots,
		txProbability,
		backoffWindowFrames);

	const double success = successPerSlotSa(g);
	const double collision = collisionProbabilitySa(g);

	const double delayFrames = predictedDelayFrames(
		predictedReadyUsers,
		raSlots,
		txProbability,
		backoffWindowFrames);

	const double delayPenalty = std::min(
		1.0,
		delayFrames / std::max(_config.tMaxFrames, _config.epsilon));

	return
		_config.alphaSuccess * success
		- _config.betaCollision * collision
		- _config.gammaDelay * delayPenalty;
}

double SimpleMarginalUtilityBasedLoadController::smoothProbability(double current, double target) const
{
	const double alpha = std::max(std::min(_config.alphaProbability, 1.0), 0.0);
	const double blended = current + alpha * (target - current);

	const double delta = blended - current;
	const double limitedDelta = std::max(
		std::min(delta, _config.maxProbabilityStep),
		-_config.maxProbabilityStep);

	return clampProbability(current + limitedDelta);
}

std::uint32_t SimpleMarginalUtilityBasedLoadController::smoothBackoff(
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

std::vector<std::uint32_t> SimpleMarginalUtilityBasedLoadController::candidateBackoffWindows(
	std::uint32_t current) const
{
	std::vector<std::uint32_t> result;
	result.reserve(_config.backoffCandidates.size() + 1);

	result.push_back(clampBackoff(current));

	for (const auto value : _config.backoffCandidates)
	{
		const auto clamped = clampBackoff(value);
		if (std::find(result.begin(), result.end(), clamped) == result.end())
			result.push_back(clamped);
	}

	std::sort(result.begin(), result.end());
	result.erase(std::unique(result.begin(), result.end()), result.end());

	return result;
}

} // namespace starTopologyEmulator