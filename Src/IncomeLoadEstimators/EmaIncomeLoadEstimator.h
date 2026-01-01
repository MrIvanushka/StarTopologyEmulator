#pragma once

#include <algorithm>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"

namespace starTopologyEmulator
{

class EmaIncomeLoadEstimator : public IIncomeLoadEstimator
{
	DECLARE_METRICS("Оценка входной нагрузки (фильтр скользящего среднего)")
public:
	struct Config {
		double alphaG = 0.2;       // Коэффициент сглаживания для нагрузки (0 < alpha <= 1)
		double alphaPlr = 0.15;    // Коэффициент сглаживания для PLR (обычно меньше, чем для G)
		double collisionWeight = 2.39; // Сколько станций "в среднем" стоит за одной коллизией
	};

	explicit EmaIncomeLoadEstimator(const Config& config);

	void update(const RandomAccessFrameResult& result) override;

	double incomeLoad() const override;

	double plr() const override;

	void reset() override;

private:
	double calculateInstantG(const RandomAccessFrameResult& res) const;

	double calculateInstantPlr(const RandomAccessFrameResult& res) const;

	Config _cfg;
	double _smoothedG = 0.0;
	double _smoothedPlr = 0.0;
	bool _isFirstUpdate = true;
};

} // namespace starTopologyEmulator
