#include "MarginalUtilityFtpGenerator.h"

#include <algorithm>
#include <limits>

#include "MarginalUtilityHelpers.h"

namespace starTopologyEmulator
{

MarginalUtilityFtpGenerator::MarginalUtilityFtpGenerator(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    MarginalUtilityFtpGeneratorConfig&& config)
    : _config(std::move(config))
    , _dynamicFrameSettings(std::move(dynamicFrameSettings))
    , _predictor(std::move(predictor))
    , _backlogAccumulator(std::move(backlogAccumulator))
    , _frameCalculator(std::move(frameCalculator))
{}

StarHubPlanMessage::FtpConfig MarginalUtilityFtpGenerator::generate(std::uint64_t frame)
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

    const int totalSlots = static_cast<int>(_frameCalculator->frameConfig().slotCountInFrame);

    const auto yellowSlots = std::visit([](const auto& cfg) { return cfg.yellowSlots; }, _config);
    const auto raMin       = std::visit([](const auto& cfg) { return cfg.raMin; }, _config);
    const auto raMax       = std::visit([](const auto& cfg) { return cfg.raMax; }, _config);

    const int available      = std::max(0, totalSlots - static_cast<int>(yellowSlots));
    const int raMaxEffective = std::min<int>(static_cast<int>(raMax), available);

    int    bestR       = static_cast<int>(raMin);
    double bestUtility = -std::numeric_limits<double>::max();

    for (int r = static_cast<int>(raMin); r <= raMaxEffective; ++r)
    {
        const double rd = static_cast<double>(r);
        const double d  = static_cast<double>(available - r);
        const double u  = std::visit([&](const auto& cfg) {
            return detail::computeUtility(cfg, rd, d, nHat, ptx, wb, qDaSlots);
        }, _config);
        if (u > bestUtility)
        {
            bestUtility = u;
            bestR = r;
        }
    }

    const auto raSlots    = static_cast<std::uint8_t>(bestR);
    const auto onlineSlots = static_cast<std::uint8_t>(
        std::max(0, available - static_cast<int>(raSlots)));
    return { onlineSlots, yellowSlots, raSlots };
}

} // namespace starTopologyEmulator
