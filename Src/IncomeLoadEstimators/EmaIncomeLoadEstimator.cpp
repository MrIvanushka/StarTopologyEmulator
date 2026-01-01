#include "EmaIncomeLoadEstimator.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

EmaIncomeLoadEstimator::EmaIncomeLoadEstimator(const Config& config)
	: _cfg(config)
{
	REGISTER_METRIC(_smoothedG, "ќценка входной нагрузки");
	REGISTER_METRIC(_smoothedPlr, "ќценка PLR");
}

void EmaIncomeLoadEstimator::update(const RandomAccessFrameResult& result)
{
	if (result.totalRaSlots == 0) return;

	double instantG = calculateInstantG(result);
	double instantPlr = calculateInstantPlr(result);

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

double EmaIncomeLoadEstimator::calculateInstantG(const RandomAccessFrameResult& res) const
{
	double totalAttempts = res.successSlots + (res.collisionSlots * _cfg.collisionWeight);
	return totalAttempts / static_cast<double>(res.totalRaSlots);
}

double EmaIncomeLoadEstimator::calculateInstantPlr(const RandomAccessFrameResult& res) const
{
	uint32_t totalAttempts = res.successSlots + res.collisionSlots;
	
	if (totalAttempts == 0)
		return 0.0;

	return static_cast<double>(res.collisionSlots) / totalAttempts;
}

} // namespace starTopologyEmulator
