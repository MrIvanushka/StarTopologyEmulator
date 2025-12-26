#include "SimpleStarHubStrategy.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

StarHubPlanMessage SimpleStarHubStrategy::generate(double g, double plr)
{
        // 1. Динамическое определение количества RA-слотов
        // Если PLR растет, выделяем больше слотов под случайный доступ
        int raSlots = calculateRaSlots(g, plr);

        // 2. Расчет параметров Backoff (интерполяция)
        StarHubPlanMessage::BackoffConfig backoff;
        backoff.useExponential = true;
        backoff.exponentBase = 2.0;

        // Плавная регулировка вероятности передачи (pTx) и окна
        // Чем выше нагрузка g, тем ниже вероятность и шире окно
        double loadFactor = std::clamp(g / _cfg.heavyLoadG, 0.0, 1.0);
        double errorFactor = std::clamp(plr / (_cfg.targetPlr * 2), 0.0, 1.0);

        // Агрегированный коэффициент стресса системы (0.0 - 1.0)
        double stress = std::max(loadFactor, errorFactor);

        backoff.pTx = _cfg.maxPTx - stress * (_cfg.maxPTx - _cfg.minPTx);
        backoff.baseWindow = static_cast<uint8_t>(_cfg.minBaseWindow +
                std::round(stress * (_cfg.maxBaseWindow - _cfg.minBaseWindow)));
        backoff.maxWindow = backoff.baseWindow * 4; // Или параметризовать отдельно

        // 3. Формирование кадра
        int yellow = 0; // Можно использовать как буфер для приоритетного входа
        int online = _cfg.totalSlots - raSlots - yellow;

        return StarHubPlanMessage(online, yellow, raSlots, backoff);
}

int SimpleStarHubStrategy::calculateRaSlots(double g, double plr) {
        // Логика: если много коллизий (PLR), нужно физически больше места для RA
        double raScale = (g > 0.1) ? (g + plr) : 0.1;
        int targetRa = static_cast<int>(_cfg.totalSlots * 0.2 * raScale);
        return std::clamp(targetRa, _cfg.minRaSlots, _cfg.maxRaSlots);
}

} // namespace starTopologyEmulator
