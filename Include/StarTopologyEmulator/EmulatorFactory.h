#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"
#include "StarTopologyEmulator/IFaces/IStationStatsCollector.h"
#include "StarTopologyEmulator/Metrics/MetricSink.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT EmulatorInitData
{
	using SendFunc = std::function<void(Timestamp, std::shared_ptr<IMessage>)>;

	std::function<std::shared_ptr<IStarStation>(SendFunc, StationID, std::shared_ptr<IStationStatsCollector>)> stationFactory;
	std::function<std::shared_ptr<IStarHub>(SendFunc)> hubFactory;
	std::unique_ptr<IFrameCalculator> abonentFrameCalculator;
	std::unique_ptr<IFrameCalculator> hubFrameCalculator;
	int stationCount = 10;
	std::shared_ptr<IMetricSink> metricSink;
};

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT EmulatorFactory
{
public:
	static std::unique_ptr<IEmulator> make(EmulatorInitData = {});
};

} // namespace starTopologyEmulator
