#include "KalmanIncomeLoadEstimator.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

KalmanIncomeLoadEstimator::KalmanIncomeLoadEstimator(KalmanIncomeLoadEstimatorConfig config)
: _cfg(std::move(config))
{
        REGISTER_METRIC(_stateG.value, "Оценка входной нагрузки");
        REGISTER_METRIC(_stateG.error, "Априорная ошибка оценки входной нагрузки");
        REGISTER_METRIC(_statePlr.value, "Оценка PLR");
        REGISTER_METRIC(_statePlr.error, "Априорная ошибка оценки PLR");
}

void KalmanIncomeLoadEstimator::update(const RandomAccessFrameResult& result)
{
        if (result.totalRaSlots == 0)
                return;

        double instG = calculateInstantG(result);
        double instPlr = calculateInstantPlr(result);

        double rCurrent = _cfg.rBase / static_cast<double>(result.totalRaSlots);


        _stateG = kalmanStep(_stateG, instG, _cfg.qG, rCurrent);
        _statePlr = kalmanStep(_statePlr, instPlr, _cfg.qPlr, rCurrent);
        
        _statePlr.value = std::clamp(_statePlr.value, 0.0, 1.0);
        _stateG.value = std::max(0.0, _stateG.value);
}

double KalmanIncomeLoadEstimator::incomeLoad() const
{
        return _stateG.value;
}

double KalmanIncomeLoadEstimator::plr() const
{
        return _statePlr.value;
}

void KalmanIncomeLoadEstimator::reset()
{
        _stateG = {0.0, 1.0};
        _statePlr = {0.0, 1.0};
}

KalmanIncomeLoadEstimator::KalmanState KalmanIncomeLoadEstimator::kalmanStep(KalmanState state, double measurement, double Q, double R)
{
        double pPred = state.error + Q;

        double kGain = pPred / (pPred + R);

        double newValue = state.value + kGain * (measurement - state.value);

        double newError = (1.0 - kGain) * pPred;

        return { newValue, newError };
}

double KalmanIncomeLoadEstimator::calculateInstantG(const RandomAccessFrameResult& res) const
{
        return (res.successSlots + (res.collisionSlots * _cfg.collisionWeight)) /
        static_cast<double>(res.totalRaSlots);
}

double KalmanIncomeLoadEstimator::calculateInstantPlr(const RandomAccessFrameResult& res) const
{
        uint32_t total = res.successSlots + res.collisionSlots;
        return (total == 0) ? 0.0 : static_cast<double>(res.collisionSlots) / total;
}

} // namespace starTopologyEmulator
