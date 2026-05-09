#include "CogorthyIncomeStationsPredictor.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

CogorthyIncomeStationsPredictor::CogorthyIncomeStationsPredictor(
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

double CogorthyIncomeStationsPredictor::estimateReadyUsers(
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
		const double impact = calculateImpact(i, targetFrame);
		total += impact;

		if (impact < 0.01 && i + 1 < currentFrame)
			break;
	}

	_scope.emit(_hReadyUsers, targetFrame, total);
	return total;
}

double CogorthyIncomeStationsPredictor::calculateImpact(
	std::uint64_t impactFrame,
	std::uint64_t targetFrame)
{
	auto planI = _dynamicFrameSettings->currentPlan(impactFrame);
	if (!planI)
		return 0.0;

	auto histIt = _incomeLoadHistory.find(impactFrame);
	if (histIt == _incomeLoadHistory.end())
		return 0.0;
	const double Gi = histIt->second;
	if (Gi <= 0.0)
		return 0.0;

	const auto& cfg = planI->backoff();
	// W — ширина диапазона задержек, которые могла выбрать станция, начавшая
	// backoff во фрейме impactFrame. При экспоненциальном backoff после
	// нескольких коллизий окно растёт до maxWindow, поэтому используем его
	// как верхнюю границу — иначе типичные горизонты планирования
	// (targetFrame - impactFrame) превышают baseWindow и формула psiAvg
	// схлопывается в 0 при pTx≈1.
	int W = cfg.useExponential
		? static_cast<int>(cfg.maxWindow)
		: static_cast<int>(cfg.baseWindow);
	if (W < 1)
		W = 1;

	const double Ci = static_cast<double>(planI->randomAccessSlotsCountInFrame())
		* Gi * (1.0 - std::exp(-Gi));

	auto planN = _dynamicFrameSettings->currentPlan(targetFrame);
	const double pN = (planN ? planN : planI)->backoff().pTx;

	double psiAvg = 0.0;
	for (int d = 1; d <= W; ++d)
	{
		const std::uint64_t s = impactFrame + static_cast<std::uint64_t>(d);
		if (s > targetFrame)
			continue;
		double prod = 1.0;
		for (std::uint64_t j = s; j + 1 <= targetFrame; ++j)
		{
			auto planJ = _dynamicFrameSettings->currentPlan(j);
			const double pj = (planJ ? planJ : planI)->backoff().pTx;
			prod *= (1.0 - pj);
		}
		psiAvg += prod;
	}
	psiAvg /= static_cast<double>(W);

	return Ci * pN * psiAvg;
}

} // namespace starTopologyEmulator
