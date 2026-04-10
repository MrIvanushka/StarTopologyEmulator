#include "StarTopologyEmulator/IncomeLoadEstimator/IncomeLoadEstimatorFactory.h"

#include "IncomeLoadEstimator/EmaIncomeLoadEstimator.h"
#include "IncomeLoadEstimator/InstantSaIncomeLoadEstimator.h"
#include "IncomeLoadEstimator/KalmanIncomeLoadEstimator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IIncomeLoadEstimator> IncomeLoadEstimatorFactory::make(EmaIncomeLoadEstimatorConfig config)
{
	auto instant = std::make_unique<InstantSaIncomeLoadEstimator>(5);
	return std::make_unique<EmaIncomeLoadEstimator>(std::move(instant), std::move(config));
}

std::unique_ptr<IIncomeLoadEstimator> IncomeLoadEstimatorFactory::make(KalmanIncomeLoadEstimatorConfig config)
{
	auto instant = std::make_unique<InstantSaIncomeLoadEstimator>(5);
	return std::make_unique<KalmanIncomeLoadEstimator>(std::move(instant), std::move(config));
}

} // namespace starTopologyEmulator
