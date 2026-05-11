#include "StarTopologyEmulator/StarHubStrategy/FtpGenerator/FtpGeneratorFactory.h"

#include "BacklogFeedbackFtpGenerator.h"
#include "LyapunovFtpGenerator.h"
#include "MarginalUtilityFtpGenerator.h"
#include "ServiceDelayFtpGenerator.h"
#include "StaticFtpGenerator.h"
#include "UtilityFtpGenerator.h"

namespace starTopologyEmulator
{

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::make(
    std::uint8_t raSlotsCount,
    std::uint8_t yellowSlotsCount,
    std::uint8_t operationSlotsCount)
{
    return std::make_unique<StaticFtpGenerator>(raSlotsCount, yellowSlotsCount, operationSlotsCount);
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeBacklogFeedback(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    BacklogFeedbackFtpGeneratorConfig&& config)
{
    return std::make_unique<BacklogFeedbackFtpGenerator>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeServiceDelay(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    ServiceDelayFtpGeneratorConfig&& config)
{
    return std::make_unique<ServiceDelayFtpGenerator>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeLyapunov(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    LyapunovFtpGeneratorConfig&& config)
{
    return std::make_unique<LyapunovFtpGenerator>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeMarginalUtility(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    MarginalUtilityFtpGeneratorConfig&& config)
{
    return std::make_unique<MarginalUtilityFtpGenerator>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeF1Linear(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    F1LinearUtilityConfig&& config)
{
    return std::make_unique<UtilityFtpGenerator<F1LinearUtilityConfig>>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeF2Logarithmic(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    F2LogarithmicUtilityConfig&& config)
{
    return std::make_unique<UtilityFtpGenerator<F2LogarithmicUtilityConfig>>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeF3AlphaFair(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    F3AlphaFairUtilityConfig&& config)
{
    return std::make_unique<UtilityFtpGenerator<F3AlphaFairUtilityConfig>>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeF4Sigmoidal(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    F4SigmoidalUtilityConfig&& config)
{
    return std::make_unique<UtilityFtpGenerator<F4SigmoidalUtilityConfig>>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeF5HardDeadline(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    F5HardDeadlineUtilityConfig&& config)
{
    return std::make_unique<UtilityFtpGenerator<F5HardDeadlineUtilityConfig>>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeF6CostOfDelay(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    F6CostOfDelayUtilityConfig&& config)
{
    return std::make_unique<UtilityFtpGenerator<F6CostOfDelayUtilityConfig>>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeF7QuadraticBacklog(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    F7QuadraticBacklogUtilityConfig&& config)
{
    return std::make_unique<UtilityFtpGenerator<F7QuadraticBacklogUtilityConfig>>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

std::unique_ptr<IFtpGenerator> FtpGeneratorFactory::makeF8Ces(
    std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings,
    std::shared_ptr<IIncomeStationsPredictor> predictor,
    std::shared_ptr<IBacklogAccumulator> backlogAccumulator,
    std::shared_ptr<IFrameCalculator> frameCalculator,
    F8CesUtilityConfig&& config)
{
    return std::make_unique<UtilityFtpGenerator<F8CesUtilityConfig>>(
        std::move(dynamicFrameSettings),
        std::move(predictor),
        std::move(backlogAccumulator),
        std::move(frameCalculator),
        std::move(config));
}

} // namespace starTopologyEmulator
