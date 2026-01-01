#pragma once

#include <algorithm>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/EmaIncomeLoadEstimatorConfig.h"

namespace starTopologyEmulator
{

class EmaIncomeLoadEstimator : public IIncomeLoadEstimator
{
	DECLARE_METRICS("ќценка входной нагрузки (фильтр скольз€щего среднего)")
public:
	explicit EmaIncomeLoadEstimator(EmaIncomeLoadEstimatorConfig);

	void update(const RandomAccessFrameResult& result) override;

	double incomeLoad() const override;

	double plr() const override;

	void reset() override;

private:
	double calculateInstantG(const RandomAccessFrameResult& res) const;

	double calculateInstantPlr(const RandomAccessFrameResult& res) const;

	EmaIncomeLoadEstimatorConfig _cfg;
	double _smoothedG = 0.0;
	double _smoothedPlr = 0.0;
	bool _isFirstUpdate = true;
};

} // namespace starTopologyEmulator
