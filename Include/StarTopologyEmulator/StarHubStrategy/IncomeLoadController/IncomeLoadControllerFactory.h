#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"
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
            StarHubPlanMessage::BackoffConfig&&);

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>, 
        HysteresisLoadControllerConfig&&);

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>, 
        SimpleMarginalUtilityBasedLoadControllerConfig&&);

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        TargetLoadControllerConfig&&);
};

} // namespace starTopologyEmulator
