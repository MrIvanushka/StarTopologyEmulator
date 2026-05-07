#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/KalmanIncomeLoadEstimatorConfig.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class KalmanIncomeLoadEstimator : public IIncomeLoadEstimator
{
public:
	KalmanIncomeLoadEstimator(
		std::unique_ptr<IIncomeLoadEstimator>,
		KalmanIncomeLoadEstimatorConfig,
		MetricScope scope = {});

	void update(const RandomAccessFrameResult& result) override;

	double incomeLoad() const override;

	double plr() const override;

	void reset() override;

private:
	struct KalmanState
	{
		double value = 0.0;
		double error = 1.0;
	};

	KalmanState kalmanStep(KalmanState state, double measurement, double Q, double R);

	double calculateInstantG(const RandomAccessFrameResult& res) const;

	double calculateInstantPlr(const RandomAccessFrameResult& res) const;

	std::unique_ptr<IIncomeLoadEstimator> _instantEstimator;

	KalmanIncomeLoadEstimatorConfig _cfg;
	KalmanState _stateG{ 0.0, 1.0 };
	KalmanState _statePlr{ 0.0, 1.0 };

	MetricScope _scope;
	MetricHandle _hG = kInvalidMetricHandle;
	MetricHandle _hGError = kInvalidMetricHandle;
	MetricHandle _hPlr = kInvalidMetricHandle;
	MetricHandle _hPlrError = kInvalidMetricHandle;
};

} // namespace starTopologyEmulator
