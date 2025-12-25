#pragma once

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarStationFactory
{
public:
	static std::shared_ptr<IStarStation> make(
		std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
		StationID id,
		int messagesNeeded,
		int rttSlots,
		std::mt19937& rng);
};

} // namespace starTopologyEmulator
