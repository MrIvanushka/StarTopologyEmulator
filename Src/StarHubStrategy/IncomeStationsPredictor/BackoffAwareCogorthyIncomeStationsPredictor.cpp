#include "BackoffAwareCogorthyIncomeStationsPredictor.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

BackoffAwareCogorthyIncomeStationsPredictor::BackoffAwareCogorthyIncomeStationsPredictor(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	MetricScope scope)
	: _incomeLoadEstimator(incomeLoadEstimator)
	, _dynamicFrameSettings(dynamicFrameSettings)
	, _scope(std::move(scope))
{
	if (_scope.active())
		_hReadyUsers = _scope.registerMetric("Оценка числа входящих станций");
}

double BackoffAwareCogorthyIncomeStationsPredictor::estimateReadyUsers(
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	_incomeLoadHistory[currentFrame] = _incomeLoadEstimator->incomeLoad();

	const std::uint64_t earliestFrame = _dynamicFrameSettings->earliestPlanNumber();
	if (currentFrame < earliestFrame || targetFrame <= currentFrame)
	{
		_scope.emit(_hReadyUsers, targetFrame, 0.0);
		return 0.0;
	}

	double total = 0.0;
	for (std::uint64_t i = currentFrame + 1; i-- > earliestFrame; )
	{
		const double impact = calculateImpact(i, targetFrame, targetFrame - currentFrame);
		total += impact;

		if (impact < 0.01 && i + 1 < currentFrame)
			break;
	}

	_scope.emit(_hReadyUsers, targetFrame, total);
	return total;
}

double BackoffAwareCogorthyIncomeStationsPredictor::calculateImpact(
	std::uint64_t impactFrame,
	std::uint64_t targetFrame,
	std::uint64_t commandApplyDelay)
{
	auto impactFramePlan = _dynamicFrameSettings->currentPlan(impactFrame);
	if (!impactFramePlan)
		return 0.0;

	const auto& backoff = impactFramePlan->backoff();
	int W = backoff.backoffType == StarHubPlanMessage::BackoffType::NONE
		? static_cast<int>(backoff.baseWindow)
		: static_cast<int>(backoff.baseWindow * backoff.exponentBase);
	if (W < 1)
		W = 1;

	const double attempts = _incomeLoadHistory[impactFrame]
		* static_cast<double>(impactFramePlan->randomAccessSlotsCountInFrame());
	const double pTx = backoff.pTx;

	double weighted = 0.0;
	for (int d = 1; d <= W; ++d)
	{
		const std::uint64_t firstTry = impactFrame + commandApplyDelay + static_cast<std::uint64_t>(d - 1);
		if (firstTry + 1 >= targetFrame)
		{
			weighted += 1.0;
			continue;
		}
		const std::uint64_t skipCount = targetFrame - firstTry - 1;
		weighted += std::pow(1.0 - pTx, static_cast<double>(skipCount));
	}

	return attempts * weighted / static_cast<double>(W);
}

} // namespace starTopologyEmulator
