#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/BacklogFeedbackFtpGeneratorConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/LyapunovFtpGeneratorConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/MarginalUtilityFtpGeneratorConfig.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/ServiceDelayFtpGeneratorConfig.h"
#include "StarTopologyEmulator/StarTopologyEmulatorExport.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT FtpGeneratorFactory
{
public:
    static std::unique_ptr<IFtpGenerator> make(
        std::uint8_t raSlotsCount,
        std::uint8_t yellowSlotsCount,
        std::uint8_t operationSlotsCount);

    static std::unique_ptr<IFtpGenerator> makeBacklogFeedback(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        BacklogFeedbackFtpGeneratorConfig&&);

    static std::unique_ptr<IFtpGenerator> makeServiceDelay(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        ServiceDelayFtpGeneratorConfig&&);

    static std::unique_ptr<IFtpGenerator> makeLyapunov(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        LyapunovFtpGeneratorConfig&&);

    static std::unique_ptr<IFtpGenerator> makeMarginalUtility(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        MarginalUtilityFtpGeneratorConfig&&);

    static std::unique_ptr<IFtpGenerator> makeF1Linear(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        F1LinearUtilityConfig&&);

    static std::unique_ptr<IFtpGenerator> makeF2Logarithmic(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        F2LogarithmicUtilityConfig&&);

    static std::unique_ptr<IFtpGenerator> makeF3AlphaFair(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        F3AlphaFairUtilityConfig&&);

    static std::unique_ptr<IFtpGenerator> makeF4Sigmoidal(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        F4SigmoidalUtilityConfig&&);

    static std::unique_ptr<IFtpGenerator> makeF5HardDeadline(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        F5HardDeadlineUtilityConfig&&);

    static std::unique_ptr<IFtpGenerator> makeF6CostOfDelay(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        F6CostOfDelayUtilityConfig&&);

    static std::unique_ptr<IFtpGenerator> makeF7QuadraticBacklog(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        F7QuadraticBacklogUtilityConfig&&);

    static std::unique_ptr<IFtpGenerator> makeF8Ces(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        F8CesUtilityConfig&&);
};

} // namespace starTopologyEmulator
