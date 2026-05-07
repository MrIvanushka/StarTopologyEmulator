#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/EmaIncomeLoadEstimatorConfig.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class EmaIncomeLoadEstimator : public IIncomeLoadEstimator
{
public:
	EmaIncomeLoadEstimator(
		std::unique_ptr<IIncomeLoadEstimator>,
		EmaIncomeLoadEstimatorConfig,
		MetricScope scope = {});

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

	MetricScope _scope;
	MetricHandle _hG = kInvalidMetricHandle;
	MetricHandle _hPlr = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
