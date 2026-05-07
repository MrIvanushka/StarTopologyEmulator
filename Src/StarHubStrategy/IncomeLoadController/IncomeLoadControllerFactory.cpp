#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/IncomeLoadControllerFactory.h"

#include "HysteresisLoadController.h"
#include "SimpleMarginalUtilityBasedLoadController.h"
#include "StaticIncomeLoadController.h"
#include "TargetLoadController.h"

namespace starTopologyEmulator
{

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	StarHubPlanMessage::BackoffConfig&& config,
	MetricScope scope)
{
	return std::make_unique<StaticIncomeLoadController>(
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IDynamicFrameSettings> dynamicSettings,
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	HysteresisLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<HysteresisLoadController>(
		dynamicSettings,
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IDynamicFrameSettings> dynamicSettings,
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	SimpleMarginalUtilityBasedLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<SimpleMarginalUtilityBasedLoadController>(
		dynamicSettings,
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IDynamicFrameSettings> dynamicSettings,
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	TargetLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<TargetLoadController>(
		dynamicSettings,
		predictor,
		std::move(config),
		std::move(scope));
}

} // namespace starTopologyEmulator
