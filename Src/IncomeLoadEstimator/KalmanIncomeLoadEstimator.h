#pragma once

#include <algorithm>
#include <memory>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/KalmanIncomeLoadEstimatorConfig.h"


namespace starTopologyEmulator
{

class KalmanIncomeLoadEstimator : public IIncomeLoadEstimator
{
	DECLARE_METRICS("Оценка входной нагрузки (фильтр Калмана)")
public:
	explicit KalmanIncomeLoadEstimator(
		std::unique_ptr<IIncomeLoadEstimator>,
		KalmanIncomeLoadEstimatorConfig);

	void update(const RandomAccessFrameResult& result) override;

	double incomeLoad() const override;

	double plr() const override;

	void reset() override;

private:
	struct KalmanState
	{
		double value = 0.0; // Оцененное значение
		double error = 1.0; // Априорная ошибка
	};

	KalmanState kalmanStep(KalmanState state, double measurement, double Q, double R);

	double calculateInstantG(const RandomAccessFrameResult& res) const;

	double calculateInstantPlr(const RandomAccessFrameResult& res) const;

	std::unique_ptr<IIncomeLoadEstimator> _instantEstimator;

	KalmanIncomeLoadEstimatorConfig _cfg;
	KalmanState _stateG{0.0, 1.0};
	KalmanState _statePlr{0.0, 1.0};
};

} // namespace starTopologyEmulator
