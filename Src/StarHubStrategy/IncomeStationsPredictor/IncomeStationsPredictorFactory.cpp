#include "StarTopologyEmulator/StarHubStrategy/IncomeStationsPredictor/IncomeStationsPredictorFactory.h"

#include "LinearRegressionIncomeStationsPredictor.h"

namespace starTopologyEmulator
{

std::unique_ptr<IIncomeStationsPredictor> IncomeStationsPredictorFactory::make(
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator,
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
	std::shared_ptr<IFrameCalculator> frameCalculator,
	LinearRegressionIncomeStationsPredictorConfig&& config)
{
	return std::make_unique<LinearRegressionIncomeStationsPredictor>(
		incomeLoadEstimator,
		dynamicFrameSettings,
		frameCalculator,
		std::move(config));
}

} // namespace starTopologyEmulator
