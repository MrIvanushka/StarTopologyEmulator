#pragma once

#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT KalmanIncomeLoadEstimatorConfig
{
	// Шум процесса: как сильно истинное значение может измениться за 1 кадр
	double qG = 0.001;
	double qPlr = 0.001;

	// Базовый шум измерения: ошибка при расчете по 1 слоту
	double rBase = 0.5;

	double collisionWeight = 2.39;
};

} // namespace starTopologyEmulator
