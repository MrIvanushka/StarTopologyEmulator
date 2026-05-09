#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IIncomeLoadController.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/Messages/StarHubPlanMessage.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/AlphaFairLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/CollisionBudgetLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/EnergyAwareLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/GreyModelAdaptiveBackoffControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/LogBarrierLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/PiLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/PseudoBayesianLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/RiskSensitiveLoadControllerConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/IncomeLoadController/SimpleMarginalUtilityBasedLoadControllerConfig.h"

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
        PiLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        SimpleMarginalUtilityBasedLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IIncomeStationsPredictor>,
        PseudoBayesianLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        CollisionBudgetLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IIncomeStationsPredictor>,
        GreyModelAdaptiveBackoffControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        LogBarrierLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        EnergyAwareLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        AlphaFairLoadControllerConfig&&,
        MetricScope scope = {});

    static std::unique_ptr<IIncomeLoadController> make(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        RiskSensitiveLoadControllerConfig&&,
        MetricScope scope = {});
};

} // namespace starTopologyEmulator
