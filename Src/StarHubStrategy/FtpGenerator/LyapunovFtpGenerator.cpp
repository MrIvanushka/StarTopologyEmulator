#include "LyapunovFtpGenerator.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace starTopologyEmulator
{

LyapunovFtpGenerator::LyapunovFtpGenerator(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    LyapunovFtpGeneratorConfig&& config)
    : _config(std::move(config))
    , _dynamicFrameSettings(std::move(dynamicFrameSettings))
    , _predictor(std::move(predictor))
    , _backlogAccumulator(std::move(backlogAccumulator))
    , _frameCalculator(std::move(frameCalculator))
{}

double LyapunovFtpGenerator::muRa(double r, double nHat, double ptx) const
{
    const double safeR = std::max(r, 1.0);
    const double g = nHat * ptx / safeR;
    return nHat * ptx * std::exp(-g);
}

double LyapunovFtpGenerator::penalty(double r, double nHat, double ptx) const
{
    const double safeR = std::max(r, 1.0);
    const double g = nHat * ptx / safeR;
    const double inefficiency = 1.0 - std::exp(-g);
    return inefficiency * inefficiency;
}

StarHubPlanMessage::FtpConfig LyapunovFtpGenerator::generate(std::uint64_t frame)
{
    const double qRa = _predictor->estimateReadyUsers(frame, frame);

    const auto currentPlan = _dynamicFrameSettings->currentPlan(frame);
    const double ptx = currentPlan ? currentPlan->backoff().pTx : 0.5;

    const double bitsPerSlot = static_cast<double>(
        std::max<std::uint64_t>(1ULL, _frameCalculator->frameConfig().bitsPerSlot));
    double totalBacklogBits = 0.0;
    for (const auto& [id, bits] : _backlogAccumulator->backlogs())
        totalBacklogBits += static_cast<double>(bits);
    const double qDa = totalBacklogBits / bitsPerSlot;

    const int totalSlots = static_cast<int>(_frameCalculator->frameConfig().slotCountInFrame);
    const int available = std::max(0, totalSlots - static_cast<int>(_config.yellowSlots));
    const int raMaxEffective = std::min<int>(_config.raMax, available);

    int bestR = _config.raMin;
    double bestCost = std::numeric_limits<double>::max();

    for (int r = _config.raMin; r <= raMaxEffective; ++r)
    {
        const double rD = static_cast<double>(r);
        const double dSlots = static_cast<double>(available - r);

        const double cost = -qRa * muRa(rD, qRa, ptx)
                          - qDa * dSlots
                          + _config.V * penalty(rD, qRa, ptx);

        if (cost < bestCost)
        {
            bestCost = cost;
            bestR = r;
        }
    }

    const auto raSlots = static_cast<std::uint8_t>(bestR);
    const auto onlineSlots = static_cast<std::uint8_t>(
        std::max(0, available - static_cast<int>(raSlots)));
    return { onlineSlots, _config.yellowSlots, raSlots };
}

} // namespace starTopologyEmulator
