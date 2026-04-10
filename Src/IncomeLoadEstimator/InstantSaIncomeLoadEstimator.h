#pragma once

#include <algorithm>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/EmaIncomeLoadEstimatorConfig.h"

namespace starTopologyEmulator
{

class InstantSaIncomeLoadEstimator : public IIncomeLoadEstimator
{
	DECLARE_METRICS("ќценка входной нагрузки (мгновенна€, S-ALOHA)")
public:
	explicit InstantSaIncomeLoadEstimator(double maxG);

	void update(const RandomAccessFrameResult& result) override;

	double incomeLoad() const override;

	double plr() const override;

	void reset() override {}

private:
	double calculateInstantG(const RandomAccessFrameResult&) const;
	double calculateInstantPlr(const RandomAccessFrameResult&) const;

	const double _maxG;

	double _instantG = 0;
	double _instantPlr = 0;
};

} // namespace starTopologyEmulator
