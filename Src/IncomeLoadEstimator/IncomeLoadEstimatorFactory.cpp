#include "StarTopologyEmulator/IncomeLoadEstimator/IncomeLoadEstimatorFactory.h"

#include "IncomeLoadEstimator/EmaIncomeLoadEstimator.h"
#include "IncomeLoadEstimator/KalmanIncomeLoadEstimator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IIncomeLoadEstimator> IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig config)
{
	return std::make_unique<EmaIncomeLoadEstimator>(std::move(config));
}

std::unique_ptr<IIncomeLoadEstimator> IncomeLoadEstimatorFactory::make(KalmanIncomeLoadEstimatorConfig config)
{
	return std::make_unique<KalmanIncomeLoadEstimator>(std::move(config));
}

} // namespace starTopologyEmulator
