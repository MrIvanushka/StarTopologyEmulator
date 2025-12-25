#include "StarTopologyEmulator/StarStationFactory.h"

#include "Stations/StarStation.h"

namespace starTopologyEmulator
{

        std::shared_ptr<IStarStation> StarStationFactory::make(
                std::function<void(Timestamp, std::shared_ptr<IMessage>)> sendFunc,
                StationID id,
                int messagesNeeded,
                int rttSlots,
                std::mt19937& rng)
        {
                return std::make_shared<StarStation>(sendFunc, id, messagesNeeded, rttSlots, rng);
        }

} // namespace starTopologyEmulator
