#include "CommonStarHubStrategy.h"

namespace starTopologyEmulator
{

CommonStarHubStrategy::CommonStarHubStrategy(
	std::unique_ptr<IFtpGenerator> ftpGenerator,
	std::unique_ptr<IIncomeLoadController> incomeLoadController,
	MetricScope scope)
	: _ftpGenerator(std::move(ftpGenerator))
	, _incomeLoadController(std::move(incomeLoadController))
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

std::shared_ptr<StarHubPlanMessage> CommonStarHubStrategy::generate(std::uint64_t currentFrame, std::uint64_t targetFrame)
{
	auto ftp = _ftpGenerator->generate(currentFrame, targetFrame);
	auto backoff = _incomeLoadController->generate(ftp.randomAccessSlotsCountInFrame, currentFrame, targetFrame);
	_lastPlan = std::make_shared<StarHubPlanMessage>(targetFrame, ftp, backoff);

	_scope.emit(_hBaseWindow, targetFrame, static_cast<double>(backoff.baseWindow));
	_scope.emit(_hMaxWindow, targetFrame, static_cast<double>(backoff.maxWindow));
	_scope.emit(_hPTx, targetFrame, backoff.pTx);
	_scope.emit(_hRaSlots, targetFrame, static_cast<double>(ftp.randomAccessSlotsCountInFrame));

	return _lastPlan;
}

} // namespace starTopologyEmulator
