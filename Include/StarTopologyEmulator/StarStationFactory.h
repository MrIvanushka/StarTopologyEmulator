#pragma once

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IFrameCalculator.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarStationFactory
{
public:
	static std::shared_ptr<IStarStation> make(
		std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
		std::unique_ptr<IFrameCalculator> frameCalculator,
		StationID id,
		int messagesNeeded,
		Timestamp tts,
		std::mt19937& rng);
};

} // namespace starTopologyEmulator
