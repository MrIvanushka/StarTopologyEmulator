#include "StaticIncomeLoadController.h"

namespace starTopologyEmulator
{

StaticIncomeLoadController::StaticIncomeLoadController(
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	StarHubPlanMessage::BackoffConfig config)
	: _readyUsersPredictor(readyUsersPredictor)
	, _config(config)
{
	REGISTER_METRIC_SUBFOLDER(_readyUsersPredictor.get());
	REGISTER_METRIC(_config.pTx, "Вероятность вещания");
}

StarHubPlanMessage::BackoffConfig StaticIncomeLoadController::generate(
	std::uint64_t plannedRaSlots,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	_readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame);
	return _config;
}

} // namespace starTopologyEmulator
