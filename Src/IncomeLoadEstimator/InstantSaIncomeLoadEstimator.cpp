#include "InstantSaIncomeLoadEstimator.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

namespace
{

double logLikelihoodGrad(double g, const RandomAccessFrameResult& result)
{
	constexpr double eps = 1e-9;
	g = std::max(g, eps);

	double grad = -static_cast<double>(result.idleSlots);

	if (result.successSlots != 0)
	{
		grad += static_cast<double>(result.successSlots) * (1.0 / g - 1.0);
	}

	if (result.collisionSlots != 0)
	{
		double term = 0.0;

		if (g < 1e-4)
		{
			term = 2.0 / g - 2.0 / 3.0 + g / 18.0;
		}
		else
		{
			const double e = std::exp(-g);
			const double denom = 1.0 - (1.0 + g) * e;
			term = g * e / denom;
		}

		grad += static_cast<double>(result.collisionSlots) * term;
	}

	return grad;
}

}

InstantSaIncomeLoadEstimator::InstantSaIncomeLoadEstimator(double maxG)
	: _maxG(maxG)
{
	REGISTER_METRIC(_lastResult.idleSlots, "Входное значение свободных слотов");
	REGISTER_METRIC(_lastResult.successSlots, "Входное значение успешных слотов");
	REGISTER_METRIC(_lastResult.collisionSlots, "Входное значение коллизионных слотов");
	REGISTER_METRIC(_instantG, "Мгновенная оценка входной нагрузки");
	REGISTER_METRIC(_instantPlr, "Мгновенная оценка PLR");
}

void InstantSaIncomeLoadEstimator::update(const RandomAccessFrameResult& result)
{
	_instantG = calculateInstantG(result);
	_instantPlr = calculateInstantPlr(result);
	_lastResult = result;
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
	constexpr double eps = 1e-6;
	constexpr double gradEps = 1e-8;

	const double minG = eps;
	const double maxG = std::max(_maxG, minG);

	// Если все слоты коллизионные, максимум правдоподобия лежит на верхней границе.
	if (res.idleSlots == 0 && res.successSlots == 0)
		return maxG;

	double left = minG;
	double right = maxG;

	const double gradLeft = logLikelihoodGrad(left, res);
	if (gradLeft <= 0.0)
		return left;

	const double gradRight = logLikelihoodGrad(right, res);
	if (gradRight >= 0.0)
		return right;

	for (int i = 0; i < 64; ++i)
	{
		const double middle = 0.5 * (left + right);
		const double grad = logLikelihoodGrad(middle, res);

		if (std::abs(grad) < gradEps)
			return middle;

		if (grad > 0.0)
			left = middle;
		else
			right = middle;
	}

	return 0.5 * (left + right);
}

double InstantSaIncomeLoadEstimator::calculateInstantPlr(const RandomAccessFrameResult& res) const
{
	uint32_t totalAttempts = res.successSlots + res.collisionSlots;
	
	if (totalAttempts == 0)
		return 0.0;

	return static_cast<double>(res.collisionSlots) / totalAttempts;
}

} // namespace starTopologyEmulator
