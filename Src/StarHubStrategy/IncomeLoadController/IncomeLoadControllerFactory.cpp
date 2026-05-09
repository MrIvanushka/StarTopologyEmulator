#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/IncomeLoadControllerFactory.h"

#include "AlphaFairLoadController.h"
#include "CollisionBudgetLoadController.h"
#include "EnergyAwareLoadController.h"
#include "GreyModelAdaptiveBackoffController.h"
#include "LogBarrierLoadController.h"
#include "PiLoadController.h"
#include "PseudoBayesianLoadController.h"
#include "RiskSensitiveLoadController.h"
#include "SimpleMarginalUtilityBasedLoadController.h"
#include "StaticIncomeLoadController.h"

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
	PiLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<PiLoadController>(
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
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	PseudoBayesianLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<PseudoBayesianLoadController>(
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IDynamicFrameSettings> dynamicSettings,
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	CollisionBudgetLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<CollisionBudgetLoadController>(
		dynamicSettings,
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	GreyModelAdaptiveBackoffControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<GreyModelAdaptiveBackoffController>(
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IDynamicFrameSettings> dynamicSettings,
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	LogBarrierLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<LogBarrierLoadController>(
		dynamicSettings,
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IDynamicFrameSettings> dynamicSettings,
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	EnergyAwareLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<EnergyAwareLoadController>(
		dynamicSettings,
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IDynamicFrameSettings> dynamicSettings,
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	AlphaFairLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<AlphaFairLoadController>(
		dynamicSettings,
		predictor,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeLoadController> IncomeLoadControllerFactory::make(
	std::shared_ptr<IDynamicFrameSettings> dynamicSettings,
	std::shared_ptr<IIncomeStationsPredictor> predictor,
	RiskSensitiveLoadControllerConfig&& config,
	MetricScope scope)
{
	return std::make_unique<RiskSensitiveLoadController>(
		dynamicSettings,
		predictor,
		std::move(config),
		std::move(scope));
}

} // namespace starTopologyEmulator
