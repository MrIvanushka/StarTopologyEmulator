#include "StarTopologyEmulator/IncomeLoadEstimator/IncomeLoadEstimatorFactory.h"

#include "IncomeLoadEstimator/EmaIncomeLoadEstimator.h"
#include "IncomeLoadEstimator/InstantSaIncomeLoadEstimator.h"
#include "IncomeLoadEstimator/KalmanIncomeLoadEstimator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IIncomeLoadEstimator> IncomeLoadEstimatorFactory::make(
	EmaIncomeLoadEstimatorConfig config,
	MetricScope scope)
{
	auto instantScope = scope.child("Мгновенный (S-ALOHA)");
	auto instant = std::make_unique<InstantSaIncomeLoadEstimator>(5, std::move(instantScope));
	return std::make_unique<EmaIncomeLoadEstimator>(
		std::move(instant),
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadEstimator> IncomeLoadEstimatorFactory::make(
	KalmanIncomeLoadEstimatorConfig config,
	MetricScope scope)
{
	auto instantScope = scope.child("Мгновенный (S-ALOHA)");
	auto instant = std::make_unique<InstantSaIncomeLoadEstimator>(5, std::move(instantScope));
	return std::make_unique<KalmanIncomeLoadEstimator>(
		std::move(instant),
		std::move(config),
		std::move(scope));
}

} // namespace starTopologyEmulator
