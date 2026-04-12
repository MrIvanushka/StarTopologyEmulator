#pragma once

#include <algorithm>
#include <memory>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/EmaIncomeLoadEstimatorConfig.h"

namespace starTopologyEmulator
{

class EmaIncomeLoadEstimator : public IIncomeLoadEstimator
{
	DECLARE_METRICS("ќценка входной нагрузки (фильтр скольз€щего среднего)")
public:
	explicit EmaIncomeLoadEstimator(
		std::unique_ptr<IIncomeLoadEstimator>,
		EmaIncomeLoadEstimatorConfig);

	void update(const RandomAccessFrameResult& result) override;

	double incomeLoad() const override;

	double plr() const override;

	void reset() override;

private:
	std::unique_ptr<IIncomeLoadEstimator> _instantEstimator;

	EmaIncomeLoadEstimatorConfig _cfg;
	double _smoothedG = 0.0;
	double _smoothedPlr = 0.0;
	bool _isFirstUpdate = true;
};

} // namespace starTopologyEmulator
