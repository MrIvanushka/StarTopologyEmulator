#include "GreyModelAdaptiveBackoffController.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace starTopologyEmulator
{

GreyModelAdaptiveBackoffController::GreyModelAdaptiveBackoffController(
	std::shared_ptr<IIncomeStationsPredictor> readyUsersPredictor,
	GreyModelAdaptiveBackoffControllerConfig&& config,
	MetricScope scope)
	: _config(std::move(config))
	, _readyUsersPredictor(readyUsersPredictor)
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hPTx = _scope.registerMetric("Вероятность вещания");
		_hBackoff = _scope.registerMetric("Окно backoff");
		_hPrediction = _scope.registerMetric("GM(1,1) прогноз n̂");
	}
}

StarHubPlanMessage::BackoffConfig GreyModelAdaptiveBackoffController::generate(
	std::uint64_t plannedRaSlots,
	std::uint64_t currentFrame,
	std::uint64_t targetFrame)
{
	const double n = _readyUsersPredictor->estimateReadyUsers(currentFrame, targetFrame);
	pushObservation(n);

	const double predicted = predictNext();
	const std::uint8_t backoff = selectBackoff(predicted, plannedRaSlots);

	StarHubPlanMessage::BackoffConfig result = _config.backoffTemplate;
	result.baseWindow = backoff;

	_scope.emit(_hPTx, targetFrame, result.pTx);
	_scope.emit(_hBackoff, targetFrame, static_cast<double>(backoff));
	_scope.emit(_hPrediction, targetFrame, predicted);

	return result;
}

void GreyModelAdaptiveBackoffController::pushObservation(double value)
{
	_history.push_back(value);
	while (_history.size() > _config.historySize && !_history.empty())
		_history.pop_front();
}

double GreyModelAdaptiveBackoffController::predictNext() const
{
	const std::size_t N = _history.size();
	if (N < _config.minHistoryForPrediction)
		return _history.empty() ? 0.0 : _history.back();

	std::vector<double> cumulative(N);
	double sum = 0.0;
	for (std::size_t k = 0; k < N; ++k)
	{
		sum += _history[k];
		cumulative[k] = sum;
	}

	double C1 = 0.0;
	double C2 = 0.0;
	double D1 = 0.0;
	double D2 = 0.0;
	for (std::size_t k = 1; k < N; ++k)
	{
		const double z = 0.5 * (cumulative[k] + cumulative[k - 1]);
		const double y = _history[k];
		C1 += z;
		C2 += z * z;
		D1 += y;
		D2 += y * z;
	}

	const double m = static_cast<double>(N - 1);
	const double det = C2 * m - C1 * C1;
	if (std::abs(det) < _config.epsilon)
		return _history.back();

	const double a = (-m * D2 + C1 * D1) / det;
	const double b = (-C1 * D2 + C2 * D1) / det;

	if (std::abs(a) < _config.epsilon)
		return _history.back();

	const double x0 = _history.front();
	const double prediction = (1.0 - std::exp(a))
		* (x0 - b / a)
		* std::exp(-a * static_cast<double>(N));

	return std::max(0.0, prediction);
}

std::uint8_t GreyModelAdaptiveBackoffController::selectBackoff(
	double predictedReadyUsers,
	std::uint64_t plannedRaSlots) const
{
	if (_config.gTarget <= _config.epsilon || predictedReadyUsers <= _config.epsilon)
		return _config.backoffTemplate.baseWindow;

	const double r = static_cast<double>(std::max<std::uint64_t>(1ULL, plannedRaSlots));
	const double pTx = _config.backoffTemplate.pTx;
	const double rawWindow = 2.0 * pTx * predictedReadyUsers / (_config.gTarget * r) - 1.0;

	const auto rounded = static_cast<std::int64_t>(std::llround(rawWindow));
	const std::int64_t clamped = std::max<std::int64_t>(
		_config.minBackoffWindow,
		std::min<std::int64_t>(rounded, _config.maxBackoffWindow));

	return static_cast<std::uint8_t>(clamped);
}

} // namespace starTopologyEmulator
