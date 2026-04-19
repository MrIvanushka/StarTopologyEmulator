#include "CommonStarHubStrategy.h"

namespace starTopologyEmulator
{

CommonStarHubStrategy::CommonStarHubStrategy(
	std::unique_ptr<IFtpGenerator> ftpGenerator,
	std::unique_ptr<IIncomeLoadController> incomeLoadController)
	: _ftpGenerator(std::move(ftpGenerator))
	, _incomeLoadController(std::move(incomeLoadController))
{
	REGISTER_METRIC_SUBFOLDER(_ftpGenerator.get());
	REGISTER_METRIC_SUBFOLDER(_incomeLoadController.get());
	REGISTER_METRIC(_lastPlan->backoff().baseWindow, "Размер окна backoff");
	REGISTER_METRIC(_lastPlan->backoff().maxWindow, "Максимальный размер окна backoff");
	REGISTER_METRIC(_lastPlan->backoff().pTx, "Вероятность занять слот абонентом");
	REGISTER_METRIC(_lastPlan->randomAccessSlotsCountInFrame(), "Количество слотов случайного доступа");
}

std::shared_ptr<StarHubPlanMessage> CommonStarHubStrategy::generate(std::uint64_t currentFrame, std::uint64_t targetFrame)
{
	auto ftp = _ftpGenerator->generate(targetFrame);
	auto backoff = _incomeLoadController->generate(ftp.randomAccessSlotsCountInFrame, currentFrame, targetFrame);
	_lastPlan = std::make_shared<StarHubPlanMessage>(targetFrame, ftp, backoff);
	return _lastPlan;
}

} // namespace starTopologyEmulator
