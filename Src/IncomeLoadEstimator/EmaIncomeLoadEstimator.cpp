#include "EmaIncomeLoadEstimator.h"

namespace starTopologyEmulator
{

EmaIncomeLoadEstimator::EmaIncomeLoadEstimator(
	std::unique_ptr<IIncomeLoadEstimator> instantEstimator,
	EmaIncomeLoadEstimatorConfig config,
	MetricScope scope)
	: _instantEstimator(std::move(instantEstimator))
	, _cfg(std::move(config))
	, _scope(std::move(scope))
{
	if (_scope.active())
	{
		_hG = _scope.registerMetric("Сглаженная оценка входной нагрузки");
		_hPlr = _scope.registerMetric("Сглаженная оценка PLR");
	}
}

void EmaIncomeLoadEstimator::update(const RandomAccessFrameResult& result)
{
	if (result.totalRaSlots == 0) return;

	_instantEstimator->update(result);

	double instantG = _instantEstimator->incomeLoad();
	double instantPlr = _instantEstimator->plr();

	if (_isFirstUpdate)
	{
		_smoothedG = instantG;
		_smoothedPlr = instantPlr;
		_isFirstUpdate = false;
	}
	else
	{
		_smoothedG = _cfg.alphaG * instantG + (1.0 - _cfg.alphaG) * _smoothedG;
		_smoothedPlr = _cfg.alphaPlr * instantPlr + (1.0 - _cfg.alphaPlr) * _smoothedPlr;
	}

	const std::uint64_t f = result.frame;
	_scope.emit(_hG, f, _smoothedG);
	_scope.emit(_hPlr, f, _smoothedPlr);
}

double EmaIncomeLoadEstimator::incomeLoad() const
{
	return _smoothedG;
}

double EmaIncomeLoadEstimator::plr() const
{
	return _smoothedPlr;
}

void EmaIncomeLoadEstimator::reset()
{
	_smoothedG = 0.0;
	_smoothedPlr = 0.0;
	_isFirstUpdate = true;
}

} // namespace starTopologyEmulator
