#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "StarTopologyEmulator/CommonTypedefs.h"
#include "StarTopologyEmulator/IFaces/IEmulator.h"
#include "StarTopologyEmulator/IFaces/IStarHub.h"
#include "StarTopologyEmulator/IFaces/IStarHubStrategy.h"
#include "StarTopologyEmulator/IFaces/IStarStation.h"

namespace starTopologyEmulator
{

struct STAR_TOPOLOGY_EMULATOR_LIB_EXPORT EmulatorInitData
{
    using SendFunc = std::function<void(Timestamp, std::shared_ptr<IMessage>)>;
    using RTT = int;

    std::function<std::shared_ptr<IStarStation>(SendFunc, StationID, RTT)> stationFactory;
    std::function<std::shared_ptr<IStarHub>(SendFunc, const std::vector<StationID>&)> hubFactory;
    int stationCount = 10;
    int rttSlots = 100;
};

class STAR_TOPOLOGY_EMULATOR_LIB_EXPORT EmulatorFactory
{
public:
    static std::unique_ptr<IEmulator> make(EmulatorInitData = {});
};

} // namespace starTopologyEmulator
