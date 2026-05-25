#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/MarginalUtilityFtpGeneratorConfig.h"

namespace starTopologyEmulator
{

class MarginalUtilityFtpGenerator : public IFtpGenerator
{
public:
    MarginalUtilityFtpGenerator(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        MarginalUtilityFtpGeneratorConfig&&);

    StarHubPlanMessage::FtpConfig generate(
            std::uint64_t currentFrame,
            std::uint64_t targetFrame) override;

private:
    const MarginalUtilityFtpGeneratorConfig _config;
    std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
    std::shared_ptr<IIncomeStationsPredictor> _predictor;
    std::shared_ptr<IBacklogAccumulator> _backlogAccumulator;
    std::shared_ptr<IFrameCalculator> _frameCalculator;
};

} // namespace starTopologyEmulator
