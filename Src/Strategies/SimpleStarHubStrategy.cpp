#include "SimpleStarHubStrategy.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

SimpleStarHubStrategy::SimpleStarHubStrategy(Config config) : _cfg(config)
{
        REGISTER_METRIC(_baseWindow, "Размер окна backoff");
        REGISTER_METRIC(_maxWindow, "Максимальный размер окна backoff");
        REGISTER_METRIC(_pTx, "Вероятность занять слот абонентом");
        REGISTER_METRIC(_raSlotsCount, "Количество слотов случайного доступа");
}

StarHubPlanMessage SimpleStarHubStrategy::generate(double g, double plr)
{
        _raSlotsCount = calculateRaSlots(g, plr);

        StarHubPlanMessage::BackoffConfig backoff;
        backoff.useExponential = true;
        backoff.exponentBase = 2.0;

        double loadFactor = std::clamp(g / _cfg.heavyLoadG, 0.0, 1.0);
        double errorFactor = std::clamp(plr / (_cfg.targetPlr * 2), 0.0, 1.0);

        double stress = std::max(loadFactor, errorFactor);

        _pTx = _cfg.maxPTx - stress * (_cfg.maxPTx - _cfg.minPTx);
        _baseWindow = static_cast<uint8_t>(_cfg.minBaseWindow +
                std::round(stress * (_cfg.maxBaseWindow - _cfg.minBaseWindow)));
        _maxWindow = backoff.baseWindow * 4;

        backoff.pTx = _pTx;
        backoff.baseWindow = _baseWindow;
        backoff.maxWindow = _maxWindow;

        int yellow = 0;
        int online = _cfg.totalSlots - _raSlotsCount - yellow;

        return StarHubPlanMessage(online, yellow, _raSlotsCount, backoff);
}

int SimpleStarHubStrategy::calculateRaSlots(double g, double plr) {
        double raScale = (g > 0.1) ? (g + plr) : 0.1;
        auto targetRa = static_cast<std::uint8_t>(_cfg.totalSlots * 0.2 * raScale);
        return std::clamp(targetRa, _cfg.minRaSlots, _cfg.maxRaSlots);
}

} // namespace starTopologyEmulator
