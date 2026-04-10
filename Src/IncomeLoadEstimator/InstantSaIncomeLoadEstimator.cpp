#include "InstantSaIncomeLoadEstimator.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

namespace
{

double logLikehoodGrad(double g, const RandomAccessFrameResult& result)
{
	auto e = exp(-g);
	return -result.idleSlots +
		result.successSlots * (1 / g - 1) +
		result.collisionSlots * (g * e / (1 - (1 + g) * e));
}

}

InstantSaIncomeLoadEstimator::InstantSaIncomeLoadEstimator(double maxG)
	: _maxG(maxG)
{
	REGISTER_METRIC(_instantG, "Мгновенная оценка входной нагрузки");
	REGISTER_METRIC(_instantPlr, "Мгновенная оценка PLR");
}

void InstantSaIncomeLoadEstimator::update(const RandomAccessFrameResult& result)
{
	_instantG = calculateInstantG(result);
	_instantPlr = calculateInstantPlr(result);
}

double InstantSaIncomeLoadEstimator::incomeLoad() const
{
	return _instantG;
}

double InstantSaIncomeLoadEstimator::plr() const
{
	return _instantPlr;
}

double InstantSaIncomeLoadEstimator::calculateInstantG(const RandomAccessFrameResult& res) const
{
	if (res.idleSlots == 0 && res.successSlots == 0)
		return 3;

	double middleG = 2.5;
	for (double step = middleG; step > 0.001; step /= 2)
	{
		auto grad = logLikehoodGrad(middleG, res);
		
		if (!grad)
			return middleG;

		middleG += step * grad / abs(grad);
	}
	return middleG;
}

double InstantSaIncomeLoadEstimator::calculateInstantPlr(const RandomAccessFrameResult& res) const
{
	uint32_t totalAttempts = res.successSlots + res.collisionSlots;
	
	if (totalAttempts == 0)
		return 0.0;

	return static_cast<double>(res.collisionSlots) / totalAttempts;
}

} // namespace starTopologyEmulator
