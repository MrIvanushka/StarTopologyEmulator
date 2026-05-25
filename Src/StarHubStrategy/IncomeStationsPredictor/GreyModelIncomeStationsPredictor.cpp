#include "GreyModelIncomeStationsPredictor.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace starTopologyEmulator
{

GreyModelIncomeStationsPredictor::GreyModelIncomeStationsPredictor(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	GreyModelIncomeStationsPredictorConfig&& config,
	MetricScope scope)
	: _config(config)
	, _incomeLoadEstimator(incomeLoadEstimator)
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _scope(std::move(scope))
{
	if (_scope.active())
		_hReadyUsers = _scope.registerMetric("Оценка количества станций в RA");
}

double GreyModelIncomeStationsPredictor::estimateReadyUsers(
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	_incomeLoadHistory[currentFrame] = _incomeLoadEstimator->incomeLoad();

	auto emitAndReturn = [&](double v) {
		const double clamped = std::max(0.0, v);
		_scope.emit(_hReadyUsers, targetFrame, clamped);
		return clamped;
	};

	const std::uint64_t earliestFrame = _dynamicFrameSettings->earliestPlanNumber();
	if (currentFrame < earliestFrame || targetFrame <= currentFrame)
		return emitAndReturn(0.0);

	const std::uint64_t windowSize = std::max<std::uint64_t>(
		1ULL,
		static_cast<std::uint64_t>(_config.windowSize));

	const std::uint64_t rawFirstFrame =
		(currentFrame + 1 > windowSize)
		? (currentFrame + 1 - windowSize)
		: 0ULL;

	const std::uint64_t firstFrame = std::max(earliestFrame, rawFirstFrame);

	std::vector<double> series;
	series.reserve(static_cast<std::size_t>(currentFrame - firstFrame + 1));

	for (std::uint64_t frame = firstFrame; frame <= currentFrame; ++frame)
	{
		const auto& plan = _dynamicFrameSettings->currentPlan(frame);
		if (!plan)
			continue;

		const auto readyUsers = historyReadyUsers(plan);
		if (!readyUsers.has_value())
			continue;

		series.push_back(std::max(0.0, *readyUsers));
	}

	if (series.empty())
		return emitAndReturn(0.0);

	if (series.size() == 1)
		return emitAndReturn(series.front());

	const std::size_t minHistory = std::max<std::size_t>(
		2U,
		static_cast<std::size_t>(_config.minHistory));

	if (series.size() < minHistory)
		return emitAndReturn(series.back());

	const std::size_t n = series.size();

	std::vector<double> x1(n);
	x1[0] = series[0];
	for (std::size_t i = 1; i < n; ++i)
		x1[i] = x1[i - 1] + series[i];

	const std::size_t m = n - 1;
	double sumZ = 0.0;
	double sumZ2 = 0.0;
	double sumY = 0.0;
	double sumZY = 0.0;

	for (std::size_t i = 0; i < m; ++i)
	{
		const double z = 0.5 * (x1[i + 1] + x1[i]);
		const double y = series[i + 1];

		sumZ  += z;
		sumZ2 += z * z;
		sumY  += y;
		sumZY += z * y;
	}

	const double mD = static_cast<double>(m);
	const double det = sumZ2 * mD - sumZ * sumZ;

	if (std::abs(det) <= _config.epsilon)
		return emitAndReturn(series.back());

	const double a = (-mD * sumZY + sumZ * sumY) / det;
	const double b = (-sumZ * sumZY + sumZ2 * sumY) / det;

	if (std::abs(a) <= _config.epsilon)
		return emitAndReturn(series.back());

	const std::uint64_t horizon = targetFrame - currentFrame;
	const std::uint64_t kIndex = static_cast<std::uint64_t>(n - 1) + horizon;
	const double exponent = -a * static_cast<double>(kIndex);

	if (exponent > 700.0 || exponent < -700.0)
		return emitAndReturn(series.back());

	const double predicted =
		std::min((1.0 - std::exp(a)) * (series[0] - b / a) * std::exp(exponent), 100.0);

	if (!std::isfinite(predicted))
		return emitAndReturn(series.back());

	return emitAndReturn(predicted);
}

double GreyModelIncomeStationsPredictor::clampProbability(double value) const
{
	return std::max(_config.minProbability, std::min(value, _config.maxProbability));
}

double GreyModelIncomeStationsPredictor::backoffEligibilityFactor(std::uint32_t windowFrames) const
{
	const std::uint32_t w = std::max<std::uint32_t>(1U, windowFrames);
	return 2.0 / (static_cast<double>(w) + 1.0);
}

double GreyModelIncomeStationsPredictor::effectiveAggressiveness(
	double txProbability,
	std::uint32_t backoffWindowFrames) const
{
	const double p = clampProbability(txProbability);
	const double eligibility = backoffEligibilityFactor(backoffWindowFrames);
	return std::max(_config.epsilon, p * eligibility);
}

std::optional<double> GreyModelIncomeStationsPredictor::historyReadyUsers(
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
