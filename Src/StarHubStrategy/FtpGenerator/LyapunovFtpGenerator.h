#pragma once

#include <memory>

#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"
#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/LyapunovFtpGeneratorConfig.h"

namespace starTopologyEmulator
{

class LyapunovFtpGenerator : public IFtpGenerator
{
public:
    LyapunovFtpGenerator(
        std::shared_ptr<IDynamicFrameSettings>,
        std::shared_ptr<IIncomeStationsPredictor>,
        std::shared_ptr<IBacklogAccumulator>,
        std::shared_ptr<IFrameCalculator>,
        LyapunovFtpGeneratorConfig&&);

    StarHubPlanMessage::FtpConfig generate(std::uint64_t frame) override;

private:
    double muRa(double r, double nHat, double ptx) const;
    double penalty(double r, double nHat, double ptx) const;

    const LyapunovFtpGeneratorConfig _config;
    std::shared_ptr<IDynamicFrameSettings> _dynamicFrameSettings;
    std::shared_ptr<IIncomeStationsPredictor> _predictor;
    std::shared_ptr<IBacklogAccumulator> _backlogAccumulator;
    std::shared_ptr<IFrameCalculator> _frameCalculator;
};

} // namespace starTopologyEmulator
