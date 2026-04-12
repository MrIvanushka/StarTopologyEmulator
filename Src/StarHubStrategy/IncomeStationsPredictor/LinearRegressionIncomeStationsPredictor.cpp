#include "LinearRegressionIncomeStationsPredictor.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

LinearRegressionIncomeStationsPredictor::LinearRegressionIncomeStationsPredictor(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IFrameCalculator> frameCalculator,
	LinearRegressionIncomeStationsPredictorConfig&& config)
	: _config(config)
	, _incomeLoadEstimator(incomeLoadEstimator)
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _frameCalculator(frameCalculator)
{}

double LinearRegressionIncomeStationsPredictor::estimateReadyUsers(
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	_incomeLoadHistory[currentFrame] = _incomeLoadEstimator->incomeLoad();
	const std::uint64_t earliestFrame = _dynamicFrameSettings->earliestPlanNumber();
	if (currentFrame < earliestFrame)
		return 0.0;

	const std::uint64_t historyCount = std::max<std::uint64_t>(
		1ULL,
		static_cast<std::uint64_t>(_config.regressionWindow));

	const std::uint64_t rawFirstFrame =
		(currentFrame + 1 > historyCount)
		? (currentFrame + 1 - historyCount)
		: 0ULL;

	const std::uint64_t firstFrame = std::max(earliestFrame, rawFirstFrame);

	struct RegressionPoint
	{
		double time = 0.0;
		double users = 0.0;
		double weight = 0.0;
	};

	std::vector<RegressionPoint> points;
	points.reserve(static_cast<std::size_t>(currentFrame - firstFrame + 1));

	const double nowTime = _frameCalculator->slotBeginTime(currentFrame, 0);
	const double targetTime = _frameCalculator->slotBeginTime(targetFrame, 0);
	const double tau = std::max(_config.epsilon, _config.forgettingHorizonSec);

	for (std::uint64_t frame = firstFrame; frame <= currentFrame; ++frame)
	{
		const auto& plan = _dynamicFrameSettings->currentPlan(frame);
		if (!plan)
			continue;

		const auto readyUsers = historyReadyUsers(plan);
		if (!readyUsers.has_value())
			continue;

		const double t = _frameCalculator->slotBeginTime(frame, 0);
		const double age = std::max(0.0, nowTime - t);
		const double w = std::exp(-age / tau);

		points.push_back({
			t,
			*readyUsers,
			w
			});
	}

	if (points.empty())
		return 0.0;

	if (points.size() == 1)
		return std::max(0.0, points.front().users);

	double sumW = 0.0;
	double sumWT = 0.0;
	double sumWY = 0.0;

	for (const auto& point : points)
	{
		sumW += point.weight;
		sumWT += point.weight * point.time;
		sumWY += point.weight * point.users;
	}

	if (sumW <= _config.epsilon)
		return std::max(0.0, points.back().users);

	const double meanT = sumWT / sumW;
	const double meanY = sumWY / sumW;

	double numerator = 0.0;
	double denominator = 0.0;

	for (const auto& point : points)
	{
		const double dt = point.time - meanT;
		const double dy = point.users - meanY;

		numerator += point.weight * dt * dy;
		denominator += point.weight * dt * dt;
	}

	double slope = 0.0;
	if (denominator > _config.epsilon)
		slope = numerator / denominator;

	const double intercept = meanY - slope * meanT;
	const double predicted = intercept + slope * targetTime;

	if (!std::isfinite(predicted))
		return std::max(0.0, points.back().users);

	return std::max(0.0, predicted);
}

double LinearRegressionIncomeStationsPredictor::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

double LinearRegressionIncomeStationsPredictor::backoffEligibilityFactor(std::uint32_t windowFrames) const
{
	const std::uint32_t w = std::max<std::uint32_t>(1U, windowFrames);
	return 2.0 / (static_cast<double>(w) + 1.0);
}

double LinearRegressionIncomeStationsPredictor::effectiveAggressiveness(
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double p = clampProbability(txProbability);
	const double eligibility = backoffEligibilityFactor(backoffWindowFrames);
	return std::max(_config.epsilon, p * eligibility);
}

std::optional<double> LinearRegressionIncomeStationsPredictor::historyReadyUsers(
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
