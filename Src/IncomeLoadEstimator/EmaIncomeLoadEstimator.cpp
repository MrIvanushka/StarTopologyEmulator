#include "EmaIncomeLoadEstimator.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

EmaIncomeLoadEstimator::EmaIncomeLoadEstimator(
	std::unique_ptr<IIncomeLoadEstimator> instantEstimator,
	EmaIncomeLoadEstimatorConfig config)
	: _instantEstimator(std::move(instantEstimator))
	, _cfg(std::move(config))
{
	REGISTER_METRIC_SUBFOLDER(_instantEstimator.get());
	REGISTER_METRIC(_smoothedG, "Сглаженная оценка входной нагрузки");
	REGISTER_METRIC(_smoothedPlr, "Сглаженная оценка PLR");
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
