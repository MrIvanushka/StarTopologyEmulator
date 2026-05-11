#include "StarTopologyEmulator/StarHubStrategy/IncomeStationsPredictor/IncomeStationsPredictorFactory.h"

#include "BackoffAwareCogorthyIncomeStationsPredictor.h"
#include "CogorthyIncomeStationsPredictor.h"
#include "LinearRegressionIncomeStationsPredictor.h"

namespace starTopologyEmulator
{

std::unique_ptr<IIncomeStationsPredictor> IncomeStationsPredictorFactory::make(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IFrameCalculator> frameCalculator,
	LinearRegressionIncomeStationsPredictorConfig&& config,
	MetricScope scope)
{
	return std::make_unique<LinearRegressionIncomeStationsPredictor>(
		incomeLoadEstimator,
		dynamicFrameSettings,
		frameCalculator,
		std::move(config),
		std::move(scope));
}

std::unique_ptr<IIncomeStationsPredictor> IncomeStationsPredictorFactory::make(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	MetricScope scope)
{
	return std::make_unique<CogorthyIncomeStationsPredictor>(
		incomeLoadEstimator,
		dynamicFrameSettings,
		std::move(scope));
}

std::unique_ptr<IIncomeStationsPredictor> IncomeStationsPredictorFactory::make(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	BackoffAwareCogorthyIncomeStationsPredictorConfig&&,
	MetricScope scope)
{
	return std::make_unique<BackoffAwareCogorthyIncomeStationsPredictor>(
		incomeLoadEstimator,
		dynamicFrameSettings,
		std::move(scope));
}

} // namespace starTopologyEmulator
