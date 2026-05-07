#include "StarTopologyEmulator/StarHubStrategy/StarHubStrategyFactory.h"

#include "StarHubStrategy/CommonStarHubStrategy.h"
#include "StarHubStrategy/SimpleStarHubStrategy.h"

namespace starTopologyEmulator
{

std::unique_ptr<IStarHubStrategy> StarHubStrategyFactory::make(
		std::unique_ptr<IFtpGenerator> ftpGenerator,
		std::unique_ptr<IIncomeLoadController> incomeLoadController,
		MetricScope scope)
{
	return std::make_unique<CommonStarHubStrategy>(
		std::move(ftpGenerator),
		std::move(incomeLoadController),
		std::move(scope));
}

std::unique_ptr<IStarHubStrategy> StarHubStrategyFactory::make(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	StarHubStrategyConfig&& config,
	MetricScope scope)
{
	return std::make_unique<SimpleStarHubStrategy>(
		incomeLoadEstimator,
		std::move(config),
		std::move(scope));
}

} // namespace starTopologyEmulator
