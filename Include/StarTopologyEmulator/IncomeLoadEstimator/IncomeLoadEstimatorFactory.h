#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/EmaIncomeLoadEstimatorConfig.h"
#include "StarTopologyEmulator/IncomeLoadEstimator/KalmanIncomeLoadEstimatorConfig.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT IncomeLoadEstimatorFactory
{
public:
	static std::unique_ptr<IIncomeLoadEstimator> make(EmaIncomeLoadEstimatorConfig, MetricScope scope = {});

	static std::unique_ptr<IIncomeLoadEstimator> make(KalmanIncomeLoadEstimatorConfig, MetricScope scope = {});
};

} // namespace starTopologyEmulator
