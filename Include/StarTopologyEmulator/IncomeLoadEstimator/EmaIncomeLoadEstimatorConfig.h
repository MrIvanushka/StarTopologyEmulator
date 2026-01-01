#pragma once

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT EmaIncomeLoadEstimatorConfig
{
	double alphaG = 0.2;       // Коэффициент сглаживания для нагрузки (0 < alpha <= 1)
	double alphaPlr = 0.15;    // Коэффициент сглаживания для PLR (обычно меньше, чем для G)
	double collisionWeight = 2.39; // Сколько станций "в среднем" стоит за одной коллизией
};

} // namespace starTopologyEmulator
