#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT StarHubFactory
{
public:
    static std::shared_ptr<IStarHub> make(
        std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
        const std::vector<StationID>& stations,
        std::unique_ptr<IStarHubStrategy> strategy);
};

} // namespace starTopologyEmulator
