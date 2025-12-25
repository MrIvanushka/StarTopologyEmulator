#include "SimpleStarHubStrategy.h"

namespace starTopologyEmulator
{

StarHubPlanMessage SimpleStarHubStrategy::generate(double g, double plr)
{
    StarHubPlanMessage::BackoffConfig cfg;

    // грубая адаптация: при больших PLR - уменьшаем pTx и увеличиваем окно
    if (plr > 0.3 || g > 0.9)
    {
        cfg.baseWindow = 4;
        cfg.maxWindow = 32;
        cfg.pTx = 0.4;
    }
    else
    {
        cfg.baseWindow = 2;
        cfg.maxWindow = 16;
        cfg.pTx = 1.0;
    }

    // для примера: 10% RA-слотов, 0% жёлтой зоны, остальное - под данные
    int total = 100;
    int ra = 10;
    int yellow = 0;
    int online = total - ra - yellow;

    return StarHubPlanMessage(online, yellow, ra, cfg);
}

} // namespace starTopologyEmulator
