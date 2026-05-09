#include "PseudoBayesianLoadController.h"

#include <algorithm>

namespace starTopologyEmulator
{

PseudoBayesianLoadController::PseudoBayesianLoadController(
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	PseudoBayesianLoadControllerConfig&& config,
	MetricScope scope)
	: _config(std::move(config))
	, _readyUsersPredictor(readyUsersPredictor)
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hPTx = _scope.registerMetric("Вероятность вещания");
		_hN = _scope.registerMetric("Оценка числа активных станций");
	}
}

StarHubPlanMessage::BackoffConfig PseudoBayesianLoadController::generate(
	std::uint64_t /*plannedRaSlots*/,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const double n = _readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame);
	const double rawPTx = (n > 1.0) ? (1.0 / n) : _config.maxProbability;
	const double clamped = std::max(_config.minProbability, std::min(rawPTx, _config.maxProbability));

	StarHubPlanMessage::BackoffConfig result = _config.backoffTemplate;
	result.pTx = clamped;

	_scope.emit(_hPTx, targetFrame, result.pTx);
	_scope.emit(_hN, targetFrame, n);

	return result;
}

} // namespace starTopologyEmulator
