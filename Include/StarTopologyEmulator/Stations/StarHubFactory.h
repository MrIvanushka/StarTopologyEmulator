#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IBacklogAccumulator.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IIncomeLoadEstimator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubInitData
{
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc;
	std::shared_ptr<IIncomeLoadEstimator> incomeLoadEstimator;
	std::shared_ptr<IFrameCalculator> frameCalculator;
	std::shared_ptr<IDynamicFrameSettings> dynamicFrameSettings;
	std::unique_ptr<IStarHubStrategy> strategy;
	std::shared_ptr<IBacklogAccumulator> backlogAccumulator;
	Timestamp tts = 100;
	MetricScope metricScope;
};

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubFactory
{
public:
	static std::shared_ptr<IStarHub> make(StarHubInitData&&);
};

} // namespace starTopologyEmulator
