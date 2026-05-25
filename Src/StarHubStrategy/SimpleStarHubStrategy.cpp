#include "SimpleStarHubStrategy.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

SimpleStarHubStrategy::SimpleStarHubStrategy(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	StarHubStrategyConfig&& config,
	MetricScope scope)
	: _cfg(std::move(config))
	, _incomeLoadEstimator(incomeLoadEstimator)
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hBaseWindow = _scope.registerMetric("Размер окна backoff");
		_hMaxWindow = _scope.registerMetric("Максимальный размер окна backoff");
		_hPTx = _scope.registerMetric("Вероятность выхода в эфир станций");
		_hRaSlots = _scope.registerMetric("Количество слотов случайного доступа");
	}
}

std::shared_ptr<StarHubPlanMessage> SimpleStarHubStrategy::generate(std::uint64_t currentFrame, std::uint64_t targetFrame)
{
	auto g = _incomeLoadEstimator->incomeLoad();
	auto plr = _incomeLoadEstimator->plr();
	const int raSlotsCount = calculateRaSlots(g, plr);

	StarHubPlanMessage::BackoffConfig backoff{};
	backoff.backoffType = StarHubPlanMessage::BackoffType::BEB;
	backoff.exponentBase = 2.0;

	double loadFactor = std::clamp(g / _cfg.heavyLoadG, 0.0, 1.0);
	double errorFactor = std::clamp(plr / (_cfg.targetPlr * 2), 0.0, 1.0);
	double stress = std::max(loadFactor, errorFactor);

	const double pTx = _cfg.maxPTx - stress * (_cfg.maxPTx - _cfg.minPTx);

	const double baseWindow = static_cast<double>(static_cast<std::uint8_t>(
		_cfg.minBaseWindow + std::lround(stress * (_cfg.maxBaseWindow - _cfg.minBaseWindow))));

	const double maxWindow = baseWindow * 4.0;

	backoff.pTx = pTx;
	backoff.baseWindow = static_cast<std::uint8_t>(baseWindow);
	backoff.maxWindow = maxWindow;

	int yellow = 0;
	int online = static_cast<int>(_cfg.totalSlots) - raSlotsCount - yellow;

	StarHubPlanMessage::FtpConfig ftp = { online, yellow, raSlotsCount };

	_scope.emit(_hBaseWindow, targetFrame, baseWindow);
	_scope.emit(_hMaxWindow, targetFrame, maxWindow);
	_scope.emit(_hPTx, targetFrame, pTx);
	_scope.emit(_hRaSlots, targetFrame, static_cast<double>(raSlotsCount));

	return std::make_shared<StarHubPlanMessage>(targetFrame, ftp, backoff);
}

int SimpleStarHubStrategy::calculateRaSlots(double g, double plr)
{
	double raScale = (g > 0.1) ? (g + plr) : 0.1;
	double raw = static_cast<double>(_cfg.totalSlots) * 0.2 * raScale;

	int targetRa = static_cast<int>(raw);
	return std::clamp(
		targetRa,
		static_cast<int>(_cfg.minRaSlots),
		static_cast<int>(_cfg.maxRaSlots)
	);
}

} // namespace starTopologyEmulator
