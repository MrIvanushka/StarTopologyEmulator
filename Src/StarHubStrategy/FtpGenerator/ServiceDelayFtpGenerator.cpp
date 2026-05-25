#include "ServiceDelayFtpGenerator.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace starTopologyEmulator
{

ServiceDelayFtpGenerator::ServiceDelayFtpGenerator(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    ServiceDelayFtpGeneratorConfig&& config)
    : _config(std::move(config))
    , _dynamicFrameSettings(std::move(dynamicFrameSettings))
    , _predictor(std::move(predictor))
    , _backlogAccumulator(std::move(backlogAccumulator))
    , _frameCalculator(std::move(frameCalculator))
{}

double ServiceDelayFtpGenerator::delayRa(double r, double nHat, double aggressiveness) const
{
    const double g = nHat * aggressiveness / std::max(r, 1.0);
    return _config.d0 + std::exp(g) / std::max(aggressiveness, 1e-9);
}

double ServiceDelayFtpGenerator::delayDa(double qDaSlots, double dSlots) const
{
    return qDaSlots / std::max(dSlots, 1.0);
}

StarHubPlanMessage::FtpConfig ServiceDelayFtpGenerator::generate(
        std::uint64_t currentFrame,
        std::uint64_t targetFrame)
{
    const double nHat = _predictor->estimateReadyUsers(currentFrame, targetFrame);

    const auto currentPlan = _dynamicFrameSettings->currentPlan(currentFrame);
    const double ptx = currentPlan ? currentPlan->backoff().pTx : 0.5;
    const double wb = currentPlan
        ? static_cast<double>(currentPlan->backoff().baseWindow)
        : 1.0;
    const double aggressiveness = std::max(1e-9, 2.0 * ptx / (wb + 1.0));

    const double bitsPerSlot = static_cast<double>(
        std::max<std::uint64_t>(1ULL, _frameCalculator->frameConfig().bitsPerSlot));
    double totalBacklogBits = 0.0;
    for (const auto& [id, bits] : _backlogAccumulator->backlogs())
        totalBacklogBits += static_cast<double>(bits);
    const double qDaSlots = totalBacklogBits / bitsPerSlot;

    const int totalSlots = static_cast<int>(_frameCalculator->frameConfig().slotCountInFrame);
    const int available = std::max(0, totalSlots - static_cast<int>(_config.yellowSlots));
    const int raMaxEffective = std::min<int>(_config.raMax, available);

    int bestR = _config.raMin;
    double bestCost = std::numeric_limits<double>::max();

    for (int r = _config.raMin; r <= raMaxEffective; ++r)
    {
        const double dSlots = static_cast<double>(available - r);
        const double dRa = delayRa(static_cast<double>(r), nHat, aggressiveness);
        const double dDa = delayDa(qDaSlots, dSlots);
        const double cost = std::abs(dRa - _config.lambdaRatio * dDa);
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
