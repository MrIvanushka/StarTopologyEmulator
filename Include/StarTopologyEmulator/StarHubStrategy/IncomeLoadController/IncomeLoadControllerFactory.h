#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/HysteresisLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/SimpleMarginalUtilityBasedLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/TargetLoadControllerConfig.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT IncomeLoadControllerFactory
{
public:
    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IIncomeStationsPredictor> predictor,
        StarHubPlanMessage::BackoffConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        HysteresisLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        SimpleMarginalUtilityBasedLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        TargetLoadControllerConfig&&,
        MetricScope scope = {});
};

} // namespace starTopologyEmulator
