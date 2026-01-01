#pragma once

#include <algorithm>

#include "Metrics/Metrics.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"

namespace starTopologyEmulator
{

class KalmanIncomeLoadEstimator : public IIncomeLoadEstimator
{
	DECLARE_METRICS("Оценка входной нагрузки (фильтр Калмана)")
public:
	struct Config
	{
		// Шум процесса: как сильно истинное значение может измениться за 1 кадр
		double qG = 0.001;   
		double qPlr = 0.001;

		// Базовый шум измерения: ошибка при расчете по 1 слоту
		double rBase = 0.5; 

		double collisionWeight = 2.39;
	};

	explicit KalmanIncomeLoadEstimator(const Config& config);

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

	Config _cfg;
	KalmanState _stateG{0.0, 1.0};
	KalmanState _statePlr{0.0, 1.0};
};

} // namespace starTopologyEmulator
