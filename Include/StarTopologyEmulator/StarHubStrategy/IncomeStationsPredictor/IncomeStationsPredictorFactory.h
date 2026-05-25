#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeStationsPredictor/BackoffAwareCogorthyIncomeStationsPredictorConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeStationsPredictor/GreyModelIncomeStationsPredictorConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeStationsPredictor/LinearRegressionIncomeStationsPredictorConfig.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT IncomeStationsPredictorFactory
{
public:
    static std::unique_ptr<IIncomeStationsPredictor> make(
        std::shared_ptr<IIncomeLoadEstimator>,
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IFrameCalculator>,
        LinearRegressionIncomeStationsPredictorConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeStationsPredictor> make(
        std::shared_ptr<IIncomeLoadEstimator>,
        std::shared_ptr<IDynamicFrameSettings>,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeStationsPredictor> make(
        std::shared_ptr<IIncomeLoadEstimator>,
        std::shared_ptr<IDynamicFrameSettings>,
        BackoffAwareCogorthyIncomeStationsPredictorConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeStationsPredictor> make(
        std::shared_ptr<IIncomeLoadEstimator>,
        std::shared_ptr<IDynamicFrameSettings>,
        GreyModelIncomeStationsPredictorConfig&&,
        MetricScope scope = {});
};

} // namespace starTopologyEmulator
