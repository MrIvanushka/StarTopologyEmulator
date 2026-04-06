#pragma once

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IDynamicFrameSettings.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarStationInitData
{
	std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc;
	std::unique_ptr<IFrameCalculator> frameCalculator;
	std::unique_ptr<IDynamicFrameSettings> dynamicFrameSettings;
	StationID id = 1;
	int messagesNeeded = 3;
	Timestamp tts = 100;
	std::mt19937& rng;
};

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarStationFactory
{
public:
	static std::shared_ptr<IStarStation> make(StarStationInitData&&);
};

} // namespace starTopologyEmulator
