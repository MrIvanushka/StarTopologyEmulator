#pragma once

#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/EmaIncomeLoadEstimatorConfig.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class InstantSaIncomeLoadEstimator : public IIncomeLoadEstimator
{
public:
	InstantSaIncomeLoadEstimator(double maxG, MetricScope scope = {});

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

	RandomAccessFrameResult _lastResult;

	MetricScope _scope;
	MetricHandle _hIdleSlots = kInvalidMetricHandle;
	MetricHandle _hSuccessSlots = kInvalidMetricHandle;
	MetricHandle _hCollisionSlots = kInvalidMetricHandle;
	MetricHandle _hInstantG = kInvalidMetricHandle;
	MetricHandle _hInstantPlr = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
