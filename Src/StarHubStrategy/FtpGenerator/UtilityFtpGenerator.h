#pragma once

#include <algorithm>
#include <limits>
#include <memory>

#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IFtpGenerator.h"
#include "StarTopologyEmulator/IFaces/IIncomeStationsPredictor.h"

#include "MarginalUtilityHelpers.h"

namespace starTopologyEmulator
{

template<typename Config>
class UtilityFtpGenerator : public IFtpGenerator
{
public:
    UtilityFtpGenerator(
        std::shared_ptr<IDynamicFrameSettings>     dynamicFrameSettings,
        std::shared_ptr<IIncomeStationsPredictor>  predictor,
        std::shared_ptr<IBacklogAccumulator>       backlogAccumulator,
        std::shared_ptr<IFrameCalculator>          frameCalculator,
        Config&&                                   config)
        : _config(std::move(config))
        , _dynamicFrameSettings(std::move(dynamicFrameSettings))
        , _predictor(std::move(predictor))
        , _backlogAccumulator(std::move(backlogAccumulator))
        , _frameCalculator(std::move(frameCalculator))
    {}

    StarHubPlanMessage::FtpConfig generate(std::uint64_t frame) override
    {
        const double nHat = _predictor->estimateReadyUsers(frame, frame);
        const auto   plan = _dynamicFrameSettings->currentPlan(frame);
        const double ptx  = plan ? plan->backoff().pTx : 0.5;
        const double wb   = plan ? static_cast<double>(plan->backoff().baseWindow) : 1.0;

        const double bitsPerSlot = static_cast<double>(
            std::max<std::uint64_t>(1ULL, _frameCalculator->frameConfig().bitsPerSlot));
        double totalBacklogBits = 0.0;
        for (const auto& [id, bits] : _backlogAccumulator->backlogs())
            totalBacklogBits += static_cast<double>(bits);
        const double qDaSlots = totalBacklogBits / bitsPerSlot;

        const int totalSlots     = static_cast<int>(_frameCalculator->frameConfig().slotCountInFrame);
        const int available      = std::max(0, totalSlots - static_cast<int>(_config.yellowSlots));
        const int raMaxEffective = std::min<int>(static_cast<int>(_config.raMax), available);

        int    bestR       = static_cast<int>(_config.raMin);
        double bestUtility = -std::numeric_limits<double>::max();

        for (int r = static_cast<int>(_config.raMin); r <= raMaxEffective; ++r)
        {
            const double rd = static_cast<double>(r);
            const double d  = static_cast<double>(available - r);
            const double u  = detail::computeUtility(_config, rd, d, nHat, ptx, wb, qDaSlots);
            if (u > bestUtility)
            {
                bestUtility = u;
                bestR = r;
            }
        }

        const auto raSlots    = static_cast<std::uint8_t>(bestR);
        const auto onlineSlots = static_cast<std::uint8_t>(
            std::max(0, available - static_cast<int>(raSlots)));
        return { onlineSlots, _config.yellowSlots, raSlots };
    }

private:
    Config                                    _config;
    std::shared_ptr<IDynamicFrameSettings>    _dynamicFrameSettings;
    std::shared_ptr<IIncomeStationsPredictor> _predictor;
    std::shared_ptr<IBacklogAccumulator>      _backlogAccumulator;
    std::shared_ptr<IFrameCalculator>         _frameCalculator;
};

} // namespace starTopologyEmulator
