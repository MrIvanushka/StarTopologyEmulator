#include "BacklogFeedbackFtpGenerator.h"

#include <algorithm>
#include <cmath>

namespace starTopologyEmulator
{

BacklogFeedbackFtpGenerator::BacklogFeedbackFtpGenerator(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    BacklogFeedbackFtpGeneratorConfig&& config)
    : _config(std::move(config))
    , _dynamicFrameSettings(std::move(dynamicFrameSettings))
    , _predictor(std::move(predictor))
    , _backlogAccumulator(std::move(backlogAccumulator))
    , _frameCalculator(std::move(frameCalculator))
    , _lastRaSlots(_config.R0)
{}

StarHubPlanMessage::FtpConfig BacklogFeedbackFtpGenerator::generate(
        std::uint64_t currentFrame,
        std::uint64_t targetFrame)
{
    const double jRa = _predictor->estimateReadyUsers(currentFrame, targetFrame);

    const double bitsPerSlot = static_cast<double>(
        std::max<std::uint64_t>(1ULL, _frameCalculator->frameConfig().bitsPerSlot));
    double totalBacklogBits = 0.0;
    for (const auto& [id, bits] : _backlogAccumulator->backlogs())
        totalBacklogBits += static_cast<double>(bits);
    const double qDa = totalBacklogBits / bitsPerSlot;

    const double rawRa = _config.R0
        + _config.kJ * (jRa - _config.jStar)
        - _config.kQ * (qDa - _config.qStar);

    const double step = std::clamp(rawRa - _lastRaSlots, -_config.deltaR, _config.deltaR);
    const double newRa = std::clamp(
        _lastRaSlots + step,
        static_cast<double>(_config.raMin),
        static_cast<double>(_config.raMax));
    _lastRaSlots = newRa;

    const int totalSlots = static_cast<int>(_frameCalculator->frameConfig().slotCountInFrame);
    const int available = std::max(0, totalSlots - static_cast<int>(_config.yellowSlots));
    const auto raSlots = static_cast<std::uint8_t>(
        std::clamp(static_cast<int>(std::lround(newRa)), 0, available));
    const auto onlineSlots = static_cast<std::uint8_t>(
        std::max(0, available - static_cast<int>(raSlots)));

    return { onlineSlots, _config.yellowSlots, raSlots };
}

} // namespace starTopologyEmulator
