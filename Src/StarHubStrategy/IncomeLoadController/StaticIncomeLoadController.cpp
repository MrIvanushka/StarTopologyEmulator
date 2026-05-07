#include "StaticIncomeLoadController.h"

namespace starTopologyEmulator
{

StaticIncomeLoadController::StaticIncomeLoadController(
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	StarHubPlanMessage::BackoffConfig config,
	MetricScope scope)
	: _readyUsersPredictor(readyUsersPredictor)
	, _config(config)
	, _scope(std::move(scope))
{
	if (_scope.active())
		_hPTx = _scope.registerMetric("Вероятность вещания");
}

StarHubPlanMessage::BackoffConfig StaticIncomeLoadController::generate(
	std::uint64_t plannedRaSlots,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	_readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame);
	_scope.emit(_hPTx, targetFrame, _config.pTx);
	return _config;
}

} // namespace starTopologyEmulator
